#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cstring>
#include <type_traits>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <atomic>

// ==================== 基础类型定义 ====================

// 常量类型枚举
enum class ConstantType {
    NIL,
    BOOLEAN,
    INTEGER,
    DOUBLE,
    STRING,
    TABLE,
    FUNCTION
};

// 常量值联合体
union ConstantValue {
    bool boolean;
    int64_t integer;
    double number;
    struct {
        const char* data;
        size_t length;
        uint32_t hash;
    } string;

    ConstantValue() : integer(0) {}
};

// ==================== 字符串常量优化 ====================

// 字符串驻留表 - 使用Robin Hood Hashing优化
class StringInternPool {
private:
    struct StringEntry {
        const char* data;
        size_t length;
        uint32_t hash;
        bool occupied;

        StringEntry() : data(nullptr), length(0), hash(0), occupied(false) {}
    };

    std::vector<StringEntry> table_;
    size_t size_;
    size_t capacity_;
    std::shared_mutex mutex_;

    // FNV-1a哈希算法
    uint32_t computeHash(const char* data, size_t length) const {
        const uint32_t prime = 16777619u;
        uint32_t hash = 2166136261u;

        for (size_t i = 0; i < length; ++i) {
            hash ^= static_cast<uint32_t>(data[i]);
            hash *= prime;
        }
        return hash;
    }

    // 重新哈希
    void rehash() {
        std::vector<StringEntry> old_table = std::move(table_);
        capacity_ *= 2;
        table_.resize(capacity_);
        size_ = 0;

        for (const auto& entry : old_table) {
            if (entry.occupied) {
                insertInternal(entry.data, entry.length, entry.hash);
            }
        }
    }

    // 内部插入
    void insertInternal(const char* data, size_t length, uint32_t hash) {
        size_t index = hash % capacity_;
        size_t probe_distance = 0;

        while (true) {
            if (!table_[index].occupied) {
                // 分配内存并复制字符串
                char* stored_data = new char[length + 1];
                std::memcpy(stored_data, data, length);
                stored_data[length] = '\0';

                table_[index] = {stored_data, length, hash, true};
                ++size_;
                return;
            }

            // Robin Hood: 如果当前探测距离小于已有项的探测距离，则交换
            size_t existing_distance = (index - (table_[index].hash % capacity_)) % capacity_;
            if (probe_distance > existing_distance) {
                std::swap(table_[index].data, const_cast<char*&>(data));
                std::swap(table_[index].length, length);
                std::swap(table_[index].hash, hash);
                probe_distance = existing_distance;
            }

            index = (index + 1) % capacity_;
            ++probe_distance;

            // 如果负载因子过高，重新哈希
            if (size_ * 4 > capacity_ * 3) {
                rehash();
                insertInternal(data, length, hash);
                return;
            }
        }
    }

public:
    StringInternPool() : size_(0), capacity_(16) {
        table_.resize(capacity_);
    }

    ~StringInternPool() {
        for (auto& entry : table_) {
            if (entry.occupied) {
                delete[] entry.data;
            }
        }
    }

    // 获取或插入字符串
    const char* intern(const char* data, size_t length) {
        std::unique_lock lock(mutex_);

        uint32_t hash = computeHash(data, length);
        size_t index = hash % capacity_;
        size_t probe_distance = 0;

        while (true) {
            if (!table_[index].occupied) {
                // 未找到，插入新字符串
                insertInternal(data, length, hash);
                return table_[index].data;
            }

            // 检查是否已存在
            if (table_[index].hash == hash &&
                table_[index].length == length &&
                std::memcmp(table_[index].data, data, length) == 0) {
                return table_[index].data;
            }

            index = (index + 1) % capacity_;
            ++probe_distance;
        }
    }

    // 基于C字符串的便捷方法
    const char* intern(const char* cstr) {
        return intern(cstr, std::strlen(cstr));
    }

    // 基于std::string的便捷方法
    const char* intern(const std::string& str) {
        return intern(str.data(), str.length());
    }
};

// ==================== 常量对象 ====================

// 常量对象基类
class ConstantObject {
public:
    virtual ~ConstantObject() = default;
    virtual ConstantType getType() const = 0;
    virtual size_t getMemoryUsage() const = 0;
    virtual bool equals(const ConstantObject* other) const = 0;
    virtual uint64_t hash() const = 0;
};

// 字符串常量
class StringConstant : public ConstantObject {
private:
    const char* data_;
    size_t length_;
    uint32_t hash_;

public:
    StringConstant(const char* data, size_t length, uint32_t hash)
        : data_(data), length_(length), hash_(hash) {}

    ConstantType getType() const override { return ConstantType::STRING; }

    size_t getMemoryUsage() const override {
        return sizeof(StringConstant) + length_ + 1;
    }

    bool equals(const ConstantObject* other) const override {
        if (other->getType() != ConstantType::STRING) return false;
        const StringConstant* str_other = static_cast<const StringConstant*>(other);
        return length_ == str_other->length_ &&
               std::memcmp(data_, str_other->data_, length_) == 0;
    }

    uint64_t hash() const override { return hash_; }

    const char* getData() const { return data_; }
    size_t getLength() const { return length_; }
};

// 表常量
class TableConstant : public ConstantObject {
private:
    std::unordered_map<uint64_t, std::shared_ptr<ConstantObject>> fields_;

public:
    ConstantType getType() const override { return ConstantType::TABLE; }

    size_t getMemoryUsage() const override {
        size_t usage = sizeof(TableConstant);
        for (const auto& pair : fields_) {
            usage += pair.second->getMemoryUsage();
        }
        return usage;
    }

    bool equals(const ConstantObject* other) const override {
        if (other->getType() != ConstantType::TABLE) return false;
        const TableConstant* table_other = static_cast<const TableConstant*>(other);

        if (fields_.size() != table_other->fields_.size()) return false;

        for (const auto& pair : fields_) {
            auto it = table_other->fields_.find(pair.first);
            if (it == table_other->fields_.end() || !pair.second->equals(it->second.get())) {
                return false;
            }
        }
        return true;
    }

    uint64_t hash() const override {
        uint64_t h = 14695981039346656037ULL;
        for (const auto& pair : fields_) {
            h ^= pair.first;
            h *= 1099511628211ULL;
            h ^= pair.second->hash();
            h *= 1099511628211ULL;
        }
        return h;
    }

    void setField(uint64_t key, std::shared_ptr<ConstantObject> value) {
        fields_[key] = value;
    }

    std::shared_ptr<ConstantObject> getField(uint64_t key) const {
        auto it = fields_.find(key);
        return it != fields_.end() ? it->second : nullptr;
    }
};

// ==================== 高性能常量池 ====================

class HighPerformanceConstantPool {
private:
    // 常量存储
    std::vector<ConstantValue> simple_constants_;
    std::vector<std::shared_ptr<ConstantObject>> complex_constants_;

    // 字符串驻留池
    StringInternPool string_pool_;

    // 常量去重映射
    std::unordered_map<uint64_t, std::vector<size_t>> constant_map_;

    // 内存使用统计
    std::atomic<size_t> memory_usage_{0};

    // 线程安全
    mutable std::shared_mutex mutex_;

    // 添加简单常量（整数、布尔值等）
    size_t addSimpleConstant(const ConstantValue& value, ConstantType type) {
        std::unique_lock lock(mutex_);

        // 对于简单值，我们直接比较（由于内存布局相同）
        uint64_t hash = computeSimpleHash(value, type);

        // 检查是否已存在
        auto it = constant_map_.find(hash);
        if (it != constant_map_.end()) {
            for (size_t index : it->second) {
                if (index < simple_constants_.size() &&
                    compareSimpleConstants(simple_constants_[index], value, type)) {
                    return index;
                }
            }
        }

        // 添加新常量
        size_t index = simple_constants_.size();
        simple_constants_.push_back(value);
        constant_map_[hash].push_back(index);

        // 更新内存使用
        memory_usage_.fetch_add(sizeof(ConstantValue), std::memory_order_relaxed);

        return index;
    }

    // 添加复杂常量
    size_t addComplexConstant(std::shared_ptr<ConstantObject> constant) {
        std::unique_lock lock(mutex_);

        uint64_t hash = constant->hash();

        // 检查是否已存在
        auto it = constant_map_.find(hash);
        if (it != constant_map_.end()) {
            for (size_t index : it->second) {
                if (index >= simple_constants_.size()) {
                    size_t complex_index = index - simple_constants_.size();
                    if (complex_constants_[complex_index]->equals(constant.get())) {
                        return index;
                    }
                }
            }
        }

        // 添加新常量
        size_t index = simple_constants_.size() + complex_constants_.size();
        complex_constants_.push_back(constant);
        constant_map_[hash].push_back(index);

        // 更新内存使用
        memory_usage_.fetch_add(constant->getMemoryUsage(), std::memory_order_relaxed);

        return index;
    }

    // 计算简单常量的哈希值
    uint64_t computeSimpleHash(const ConstantValue& value, ConstantType type) const {
        switch (type) {
        case ConstantType::NIL:
            return 0;
        case ConstantType::BOOLEAN:
            return value.boolean ? 1 : 2;
        case ConstantType::INTEGER:
            return static_cast<uint64_t>(value.integer) * 11400714819323198485ULL;
        case ConstantType::DOUBLE:
            // 将double按位解释为整数进行哈希
            static_assert(sizeof(double) == sizeof(uint64_t), "double must be 64 bits");
            uint64_t bits;
            std::memcpy(&bits, &value.number, sizeof(double));
            return bits * 11400714819323198485ULL;
        default:
            return 0;
        }
    }

    // 比较简单常量
    bool compareSimpleConstants(const ConstantValue& a, const ConstantValue& b, ConstantType type) const {
        switch (type) {
        case ConstantType::NIL:
            return true; // 所有nil都相等
        case ConstantType::BOOLEAN:
            return a.boolean == b.boolean;
        case ConstantType::INTEGER:
            return a.integer == b.integer;
        case ConstantType::DOUBLE:
            return a.number == b.number;
        default:
            return false;
        }
    }

public:
    HighPerformanceConstantPool() {
        // 预分配一些空间
        simple_constants_.reserve(64);
        complex_constants_.reserve(32);
    }

    // 添加nil常量
    size_t addNil() {
        ConstantValue value;
        return addSimpleConstant(value, ConstantType::NIL);
    }

    // 添加布尔常量
    size_t addBoolean(bool b) {
        ConstantValue value;
        value.boolean = b;
        return addSimpleConstant(value, ConstantType::BOOLEAN);
    }

    // 添加整数常量
    size_t addInteger(int64_t i) {
        ConstantValue value;
        value.integer = i;
        return addSimpleConstant(value, ConstantType::INTEGER);
    }

    // 添加浮点数常量
    size_t addDouble(double d) {
        ConstantValue value;
        value.number = d;
        return addSimpleConstant(value, ConstantType::DOUBLE);
    }

    // 添加字符串常量
    size_t addString(const char* data, size_t length) {
        // 字符串驻留
        const char* interned_data = string_pool_.intern(data, length);
        uint32_t hash = 0;

        // 计算哈希（字符串池已经计算过，但这里为了演示重新计算）
        const uint32_t prime = 16777619u;
        hash = 2166136261u;
        for (size_t i = 0; i < length; ++i) {
            hash ^= static_cast<uint32_t>(interned_data[i]);
            hash *= prime;
        }

        auto string_constant = std::make_shared<StringConstant>(interned_data, length, hash);
        return addComplexConstant(string_constant);
    }

    // 便捷方法：从C字符串添加
    size_t addString(const char* cstr) {
        return addString(cstr, std::strlen(cstr));
    }

    // 便捷方法：从std::string添加
    size_t addString(const std::string& str) {
        return addString(str.data(), str.length());
    }

    // 添加表常量
    size_t addTable() {
        auto table_constant = std::make_shared<TableConstant>();
        return addComplexConstant(table_constant);
    }

    // 获取常量值
    ConstantValue getSimpleConstant(size_t index) const {
        std::shared_lock lock(mutex_);
        if (index < simple_constants_.size()) {
            return simple_constants_[index];
        }
        return ConstantValue(); // 返回默认值（错误情况）
    }

    // 获取复杂常量
    std::shared_ptr<ConstantObject> getComplexConstant(size_t index) const {
        std::shared_lock lock(mutex_);
        if (index >= simple_constants_.size()) {
            size_t complex_index = index - simple_constants_.size();
            if (complex_index < complex_constants_.size()) {
                return complex_constants_[complex_index];
            }
        }
        return nullptr;
    }

    // 获取常量类型
    ConstantType getConstantType(size_t index) const {
        if (index < simple_constants_.size()) {
            // 简单常量类型需要外部记录，这里简化处理
            // 实际实现中应该存储类型信息
            return ConstantType::NIL; // 占位符
        }

        auto constant = getComplexConstant(index);
        return constant ? constant->getType() : ConstantType::NIL;
    }

    // 获取字符串常量数据
    const char* getStringData(size_t index) const {
        auto constant = getComplexConstant(index);
        if (auto string_constant = std::dynamic_pointer_cast<StringConstant>(constant)) {
            return string_constant->getData();
        }
        return nullptr;
    }

    // 内存使用统计
    size_t getMemoryUsage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

    // 常量数量统计
    size_t getConstantCount() const {
        std::shared_lock lock(mutex_);
        return simple_constants_.size() + complex_constants_.size();
    }

    // 清理未使用的常量（标记清除）
    void cleanup() {
        std::unique_lock lock(mutex_);

        // 简化实现：在实际使用中，这里应该实现引用计数或垃圾回收
        // 这里只是重置池（演示用途）
        simple_constants_.clear();
        complex_constants_.clear();
        constant_map_.clear();
        memory_usage_.store(0, std::memory_order_relaxed);
    }
};

// ==================== 使用示例 ====================

void main_test_const_pool() {
    HighPerformanceConstantPool pool;

    std::cout << "=== 高性能常量池示例 ===" << std::endl;

    // 添加各种类型的常量
    size_t nil_index = pool.addNil();
    size_t true_index = pool.addBoolean(true);
    size_t false_index = pool.addBoolean(false);
    size_t int_index = pool.addInteger(42);
    size_t double_index = pool.addDouble(3.14159);
    size_t str1_index = pool.addString("Hello, World!");
    size_t str2_index = pool.addString("Hello, World!"); // 重复字符串
    size_t str3_index = pool.addString("Different String");

    std::cout << "添加常量后的内存使用: " << pool.getMemoryUsage() << " 字节" << std::endl;
    std::cout << "常量总数: " << pool.getConstantCount() << std::endl;

    // 测试字符串驻留
    std::cout << "\n=== 字符串驻留测试 ===" << std::endl;
    std::cout << "字符串1地址: " << (void*)pool.getStringData(str1_index) << std::endl;
    std::cout << "字符串2地址: " << (void*)pool.getStringData(str2_index) << std::endl;
    std::cout << "字符串3地址: " << (void*)pool.getStringData(str3_index) << std::endl;

    // 测试字符串内容
    std::cout << "\n=== 字符串内容测试 ===" << std::endl;
    std::cout << "字符串1: " << pool.getStringData(str1_index) << std::endl;
    std::cout << "字符串2: " << pool.getStringData(str2_index) << std::endl;
    std::cout << "字符串3: " << pool.getStringData(str3_index) << std::endl;

    // 测试常量去重
    std::cout << "\n=== 常量去重测试 ===" << std::endl;
    size_t another_int_index = pool.addInteger(42); // 重复整数
    std::cout << "原始整数索引: " << int_index << std::endl;
    std::cout << "重复整数索引: " << another_int_index << std::endl;
    std::cout << "是否相同: " << (int_index == another_int_index ? "是" : "否") << std::endl;

    // 性能测试：添加大量字符串
    std::cout << "\n=== 性能测试 ===" << std::endl;

    const int TEST_COUNT = 10000;
    std::vector<size_t> indices;
    indices.reserve(TEST_COUNT);

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_COUNT; ++i) {
        std::string test_str = "TestString_" + std::to_string(i % 100); // 很多重复
        indices.push_back(pool.addString(test_str));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "添加 " << TEST_COUNT << " 个字符串耗时: "
              << duration.count() << " 微秒" << std::endl;
    std::cout << "实际常量数量: " << pool.getConstantCount() << std::endl;
    std::cout << "内存使用: " << pool.getMemoryUsage() << " 字节" << std::endl;

    // 测试唯一字符串数量
    std::unordered_map<const char*, int> unique_strings;
    for (size_t index : indices) {
        const char* str = pool.getStringData(index);
        unique_strings[str]++;
    }
    std::cout << "唯一字符串数量: " << unique_strings.size() << std::endl;
}
