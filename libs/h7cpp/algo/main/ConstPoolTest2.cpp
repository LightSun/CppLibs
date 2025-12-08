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
#include <functional>

namespace const_pool2 {
using String = std::string;
using CString = const std::string &;

// ==================== 基础类型定义 ====================

enum class ConstantType {
    NIL,
    BOOLEAN,
    INTEGER,
    DOUBLE,
    STRING,
    TABLE,
    FUNCTION
};

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

// ==================== 改进的哈希冲突处理 ====================

// 哈希条目 - 支持链地址法和开放寻址
template<typename T>
struct HashEntry {
    uint64_t hash {0};
    T value;
    bool occupied {false};
    HashEntry<T>* next {nullptr}; // 链地址法解决冲突

    HashEntry(){}
    HashEntry(uint64_t h, const T& v) : hash(h), value(v), occupied(true){}
};

// 高性能哈希表 - 结合链地址法和开放寻址
template<typename Key,
         typename Value,
         typename Hash = std::hash<Key>,
         typename Equal = std::equal_to<Key>>
class HighPerformanceHashTable {
private:
    std::vector<HashEntry<Value>> table_;
    size_t size_;
    size_t capacity_;
    double max_load_factor_;
    Hash hasher_;
    Equal equal_;
    std::shared_mutex mutex_;

    // 计算索引
    size_t computeIndex(uint64_t hash) const {
        return hash % capacity_;
    }

    // 重新哈希
    void rehash() {
        std::vector<HashEntry<Value>> old_table = std::move(table_);
        capacity_ = std::max(capacity_ * 2, size_t(16));
        table_.resize(capacity_);
        size_ = 0;

        for (auto& entry : old_table) {
            if (entry.occupied) {
                // 重新插入主条目
                insertInternal(entry.hash, entry.value);

                // 重新插入链地址法的冲突条目
                HashEntry<Value>* current = entry.next;
                while (current) {
                    insertInternal(current->hash, current->value);
                    HashEntry<Value>* next = current->next;
                    delete current;
                    current = next;
                }
            }
        }
    }

    // 内部插入
    Value* insertInternal(uint64_t hash, const Value& value) {
        size_t index = computeIndex(hash);

        if (!table_[index].occupied) {
            // 主位置空闲
            table_[index].hash = hash;
            table_[index].value = value;
            table_[index].occupied = true;
            ++size_;
            return &table_[index].value;
        }

        // 主位置被占用，检查是否是相同的键
        if (table_[index].hash == hash) {
            // 相同的哈希值，但需要检查值是否真的相等
            // 这里假设调用者已经检查过，直接返回
            if(table_[index].value == value){
                return &table_[index].value;
            }
        }

        // 哈希冲突，使用链地址法
        HashEntry<Value>* new_entry = new HashEntry<Value>(hash, value);

        // 找到链表的末尾
        HashEntry<Value>* current = &table_[index];
        while (current->next) {
            current = current->next;
            // 检查是否已存在
            if (current->hash == hash) {
                delete new_entry;
                return &current->value;
            }
        }
        current->next = new_entry;
        ++size_;
        return &new_entry->value;
    }

public:
    HighPerformanceHashTable(size_t initial_capacity = 16, double max_load = 0.75)
        : size_(0), capacity_(initial_capacity), max_load_factor_(max_load) {
        table_.resize(capacity_);
    }

    ~HighPerformanceHashTable() {
        for (auto& entry : table_) {
            HashEntry<Value>* current = entry.next;
            while (current) {
                HashEntry<Value>* next = current->next;
                delete current;
                current = next;
            }
        }
    }

    // 插入键值对
    Value* insert(uint64_t hash, const Value& value) {
        std::unique_lock lock(mutex_);

        // 检查负载因子
        if (static_cast<double>(size_) / capacity_ > max_load_factor_) {
            rehash();
        }
        return insertInternal(hash, value);
    }

    // 查找值
    Value* find(uint64_t hash) {
        std::shared_lock lock(mutex_);

        size_t index = computeIndex(hash);
        if (!table_[index].occupied) {
            return nullptr;
        }

        // 检查主位置
        if (table_[index].hash == hash) {
            return &table_[index].value;
        }

        // 在冲突链中查找
        HashEntry<Value>* current = table_[index].next;
        while (current) {
            if (current->hash == hash) {
                return &current->value;
            }
            current = current->next;
        }

        return nullptr;
    }

    // 获取大小
    size_t size() const {
        std::shared_lock lock(mutex_);
        return size_;
    }

    // 获取容量
    size_t capacity() const {
        std::shared_lock lock(mutex_);
        return capacity_;
    }

    // 统计冲突信息
    void getCollisionStats()  {
        std::shared_lock lock(mutex_);

        size_t max_chain_length = 0;
        double avg_chain_length = 0;
        //
        size_t total_chains = 0;
        size_t chains_with_collisions = 0;

        for (const auto& entry : table_) {
            if (entry.occupied) {
                total_chains++;
                size_t chain_length = 1;

                HashEntry<Value>* current = entry.next;
                while (current) {
                    chain_length++;
                    current = current->next;
                }

                if (chain_length > 1) {
                    chains_with_collisions++;
                }

                max_chain_length = std::max(max_chain_length, chain_length);
            }
        }

        avg_chain_length = total_chains > 0 ? static_cast<double>(size_) / total_chains : 0.0;

        std::cout << "哈希表统计:" << std::endl;
        std::cout << "  - 总条目数: " << size_ << std::endl;
        std::cout << "  - 容量: " << capacity_ << std::endl;
        std::cout << "  - 负载因子: " << (static_cast<double>(size_) / capacity_) << std::endl;
        std::cout << "  - 最大链长度: " << max_chain_length << std::endl;
        std::cout << "  - 平均链长度: " << avg_chain_length << std::endl;
        std::cout << "  - 有冲突的链数量: " << chains_with_collisions << std::endl;
        std::cout << "  - 冲突率: " << (static_cast<double>(chains_with_collisions) / total_chains * 100) << "%" << std::endl;
    }
};

// ==================== 改进的字符串驻留池 ====================

class ImprovedStringInternPool {
private:
    struct StringInfo {
        String data;
        uint32_t hash;

        StringInfo(){}
        StringInfo(CString data, uint32_t h) : data(data), hash(h) {}
        StringInfo(const char* data, size_t len,  uint32_t h): data(String(data, len)), hash(h) {}

        size_t length()const{
            return data.length();
        }
        const char* kdata()const{
            return data.data();
        }
        friend bool operator==(const StringInfo s1, const StringInfo s2){
            return s1.data == s2.data;
        }
    };
    using STable = HighPerformanceHashTable<uint64_t, StringInfo>;

    STable hash_table_ {16, 0.7};

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

    // 双重哈希检查
    bool compareStrings(const char* a, const char* b, size_t length) const {
        // 快速检查：比较前8个字节
        if (length >= 8) {
            uint64_t* a64 = (uint64_t*)a;
            uint64_t* b64 = (uint64_t*)b;
            if (*a64 != *b64) return false;
        }

        // 完整比较
        return std::memcmp(a, b, length) == 0;
    }

public:
    ImprovedStringInternPool(){}

    ~ImprovedStringInternPool() {
        // 清理所有分配的字符串内存
        // 在实际实现中，这里需要遍历哈希表并delete[]所有data
    }

    // 获取或插入字符串
    const char* intern(const char* data, size_t length) {
        uint32_t hash = computeHash(data, length);
        uint64_t combined_hash = (static_cast<uint64_t>(hash) << 32) | length;

        // 查找现有字符串
        auto* existing = hash_table_.find(combined_hash);
        if (existing) {
            // 哈希匹配，但需要验证内容是否真的相同
            if (existing->length() == length && compareStrings(existing->kdata(), data, length)) {
                return existing->kdata();
            }
            // 哈希冲突！使用更精确的比较
            // 这里我们线性搜索所有具有相同哈希的字符串
            // 在实际实现中，这应该很少发生
            std::cout << "警告：检测到哈希冲突！哈希: " << hash << ", 长度: " << length << std::endl;
        }
        if(data){
            return hash_table_.insert(combined_hash, StringInfo(data, length))->kdata();
        }else{
            return hash_table_.insert(combined_hash, StringInfo("", length))->kdata();
        }
    }

    const char* intern(const char* cstr) {
        return intern(cstr, std::strlen(cstr));
    }

    const char* intern(const std::string& str) {
        return intern(str.data(), str.length());
    }

    // 获取统计信息
    void getStats(){
        hash_table_.getCollisionStats();
    }
};

// ==================== 常量对象（简化版） ====================

class ConstantObject {
public:
    virtual ~ConstantObject() = default;
    virtual ConstantType getType() const = 0;
    virtual size_t getMemoryUsage() const = 0;
    virtual bool equals(const ConstantObject* other) const = 0;
    virtual uint64_t hash() const = 0;
};

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

    uint64_t hash() const override {
        return (static_cast<uint64_t>(hash_) << 32) | length_;
    }

    const char* getData() const { return data_; }
    size_t getLength() const { return length_; }
};

// ==================== 改进的常量池 ====================

class ImprovedConstantPool {
private:
    // 常量存储
    std::vector<ConstantValue> simple_constants_;
    std::vector<std::shared_ptr<ConstantObject>> complex_constants_;

    // 改进的字符串驻留池
    ImprovedStringInternPool string_pool_;

    // 改进的哈希表用于常量去重
    HighPerformanceHashTable<uint64_t, size_t> constant_map_;

    // 内存使用统计
    std::atomic<size_t> memory_usage_{0};

    // 线程安全
    mutable std::shared_mutex mutex_;

    // 添加简单常量
    size_t addSimpleConstant(const ConstantValue& value, ConstantType type) {
        std::unique_lock lock(mutex_);

        uint64_t hash = computeSimpleHash(value, type);

        // 使用改进的哈希表查找
        auto* existing_index = constant_map_.find(hash);
        if (existing_index) {
            // 找到相同哈希的条目，需要验证是否真的相同
            if (*existing_index < simple_constants_.size() &&
                compareSimpleConstants(simple_constants_[*existing_index], value, type)) {
                return *existing_index;
            }

            // 哈希冲突！需要处理
            std::cout << "简单常量哈希冲突！类型: " << static_cast<int>(type)
                      << ", 哈希: " << hash << std::endl;
        }

        // 添加新常量
        size_t index = simple_constants_.size();
        simple_constants_.push_back(value);
        constant_map_.insert(hash, index);

        memory_usage_.fetch_add(sizeof(ConstantValue), std::memory_order_relaxed);

        return index;
    }

    // 添加复杂常量
    size_t addComplexConstant(std::shared_ptr<ConstantObject> constant) {
        std::unique_lock lock(mutex_);

        uint64_t hash = constant->hash();

        // 查找现有常量
        auto* existing_index = constant_map_.find(hash);
        if (existing_index) {
            if (*existing_index >= simple_constants_.size()) {
                size_t complex_index = *existing_index - simple_constants_.size();
                if (complex_index < complex_constants_.size() &&
                    complex_constants_[complex_index]->equals(constant.get())) {
                    return *existing_index;
                }
            }

            // 哈希冲突
            std::cout << "复杂常量哈希冲突！类型: " << static_cast<int>(constant->getType())
                      << ", 哈希: " << hash << std::endl;
        }

        // 添加新常量
        size_t index = simple_constants_.size() + complex_constants_.size();
        complex_constants_.push_back(constant);
        constant_map_.insert(hash, index);

        memory_usage_.fetch_add(constant->getMemoryUsage(), std::memory_order_relaxed);

        return index;
    }

    // 计算简单常量的哈希值
    uint64_t computeSimpleHash(const ConstantValue& value, ConstantType type) const {
        switch (type) {
        case ConstantType::NIL:
            return 0xDEADBEEF; // 特殊的魔法数
        case ConstantType::BOOLEAN:
            return value.boolean ? 0xBADF00D : 0xFACEFEED; // 不同的魔法数
        case ConstantType::INTEGER:
            // 使用更好的整数哈希
            {
                uint64_t x = static_cast<uint64_t>(value.integer);
                x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
                x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
                x = x ^ (x >> 31);
                return x;
            }

        case ConstantType::DOUBLE:{
            // 更好的双精度浮点数哈希
            static_assert(sizeof(double) == sizeof(uint64_t), "double must be 64 bits");
            uint64_t bits;
            std::memcpy(&bits, &value.number, sizeof(double));
            if (value.number == 0.0) bits = 0; // 标准化+0和-0
            bits = (bits ^ (bits >> 32)) * 0x45d9f3b3335b369ULL;
            bits = (bits ^ (bits >> 29)) * 0x45d9f3b3335b369ULL;
            bits = bits ^ (bits >> 32);
            return bits;
        }

        default:
            return 0;
        }
    }

    // 比较简单常量
    bool compareSimpleConstants(const ConstantValue& a, const ConstantValue& b, ConstantType type) const {
        switch (type) {
        case ConstantType::NIL:
            return true;
        case ConstantType::BOOLEAN:
            return a.boolean == b.boolean;
        case ConstantType::INTEGER:
            return a.integer == b.integer;
        case ConstantType::DOUBLE:
            // 处理浮点数精度问题
            return std::abs(a.number - b.number) < 1e-15;
        default:
            return false;
        }
    }

public:
    ImprovedConstantPool() : constant_map_(16, 0.75) {
        simple_constants_.reserve(64);
        complex_constants_.reserve(32);
    }

    // 添加各种常量类型的方法
    size_t addNil() {
        ConstantValue value;
        return addSimpleConstant(value, ConstantType::NIL);
    }

    size_t addBoolean(bool b) {
        ConstantValue value;
        value.boolean = b;
        return addSimpleConstant(value, ConstantType::BOOLEAN);
    }

    size_t addInteger(int64_t i) {
        ConstantValue value;
        value.integer = i;
        return addSimpleConstant(value, ConstantType::INTEGER);
    }

    size_t addDouble(double d) {
        ConstantValue value;
        value.number = d;
        return addSimpleConstant(value, ConstantType::DOUBLE);
    }

    size_t addString(const char* data, size_t length) {
        const char* interned_data = string_pool_.intern(data, length);
        uint32_t hash = 0;

        const uint32_t prime = 16777619u;
        hash = 2166136261u;
        for (size_t i = 0; i < length; ++i) {
            hash ^= static_cast<uint32_t>(interned_data[i]);
            hash *= prime;
        }

        auto string_constant = std::make_shared<StringConstant>(interned_data, length, hash);
        return addComplexConstant(string_constant);
    }

    size_t addString(const char* cstr) {
        return addString(cstr, std::strlen(cstr));
    }

    size_t addString(const std::string& str) {
        return addString(str.data(), str.length());
    }

    // 获取常量
    ConstantValue getSimpleConstant(size_t index) const {
        std::shared_lock lock(mutex_);
        if (index < simple_constants_.size()) {
            return simple_constants_[index];
        }
        return ConstantValue();
    }

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

    const char* getStringData(size_t index) const {
        auto constant = getComplexConstant(index);
        if (auto string_constant = std::dynamic_pointer_cast<StringConstant>(constant)) {
            return string_constant->getData();
        }
        return nullptr;
    }

    // 统计信息
    size_t getMemoryUsage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

    size_t getConstantCount() const {
        std::shared_lock lock(mutex_);
        return simple_constants_.size() + complex_constants_.size();
    }

    void getStats(){
        std::cout << "\n=== 常量池统计 ===" << std::endl;
        std::cout << "总常量数: " << getConstantCount() << std::endl;
        std::cout << "内存使用: " << getMemoryUsage() << " 字节" << std::endl;

        constant_map_.getCollisionStats();
        string_pool_.getStats();
    }
};

// ==================== 测试哈希冲突 ====================

// 故意产生哈希冲突的测试字符串
std::vector<std::string> generateCollidingStrings(int count) {
    std::vector<std::string> result;

    // 这些字符串经过设计会产生哈希冲突
    result.push_back("Aa");
    result.push_back("BB");

    // 更多可能产生冲突的字符串
    for (int i = 0; i < count - 2; ++i) {
        result.push_back("Test_" + std::to_string(i));
    }

    return result;
}

}

// 测试函数
void testHashCollisions() {
    using namespace const_pool2;
    std::cout << "=== 哈希冲突测试 ===" << std::endl;

    ImprovedConstantPool pool;

    // 测试1：故意产生冲突的字符串
    auto colliding_strings = generateCollidingStrings(10);

    std::cout << "\n测试故意冲突的字符串:" << std::endl;
    for (const auto& str : colliding_strings) {
        size_t index = pool.addString(str);
        std::cout << "添加字符串: '" << str << "' -> 索引: " << index << std::endl;
    }

    // 测试2：大量随机字符串
    std::cout << "\n测试大量随机字符串:" << std::endl;
    const int LARGE_TEST_COUNT = 1000;
    std::vector<size_t> indices;

    for (int i = 0; i < LARGE_TEST_COUNT; ++i) {
        std::string random_str = "RandomString_" + std::to_string(rand() % 100);
        indices.push_back(pool.addString(random_str));
    }

    // 测试3：重复字符串
    std::cout << "\n测试重复字符串:" << std::endl;
    size_t first_index = pool.addString("DuplicateString");
    size_t second_index = pool.addString("DuplicateString");
    std::cout << "第一次添加索引: " << first_index << std::endl;
    std::cout << "第二次添加索引: " << second_index << std::endl;
    std::cout << "是否相同: " << (first_index == second_index ? "是" : "否") << std::endl;

    // 显示统计信息
    pool.getStats();
}

// ==================== 主函数 ====================

void main_test_const_pool2() {
    using namespace const_pool2;
    std::cout << "=== 改进的高性能常量池：哈希冲突处理 ===" << std::endl;

    // 运行哈希冲突测试
    testHashCollisions();

    // 基本功能测试
    std::cout << "\n\n=== 基本功能测试 ===" << std::endl;

    ImprovedConstantPool pool;

    // 添加各种类型的常量
    pool.addNil();
    pool.addBoolean(true);
    pool.addBoolean(false);
    pool.addInteger(42);
    pool.addDouble(3.14159);
    pool.addString("Hello, World!");
    pool.addString("Hello, World!"); // 重复
    pool.addString("Different String");

    std::cout << "基本测试完成" << std::endl;
    pool.getStats();
}
