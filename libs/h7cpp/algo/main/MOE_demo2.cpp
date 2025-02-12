
#include <iostream>
#include <vector>

#ifdef OpenCV_DIR
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

namespace moe2 {
// 基类：特征提取器
class FeatureExtractor {
public:
    virtual vector<float> extract(const Mat& img) = 0;
    virtual ~FeatureExtractor() {}
};

// 颜色专家：HSV直方图
class ColorExpert : public FeatureExtractor {
public:
    vector<float> extract(const Mat& img) override {
        Mat hsv, hist;
        cvtColor(img, hsv, COLOR_BGR2HSV);

        // 计算H通道直方图（色调）
        int channels[] = {0};
        int histSize[] = {16};
        float range[] = {0, 180};
        const float* ranges[] = {range};

        calcHist(&hsv, 1, channels, Mat(), hist, 1, histSize, ranges);
        normalize(hist, hist, 1, 0, NORM_L1);

        return vector<float>(hist.begin<float>(), hist.end<float>());
    }
};

// 纹理专家：GLCM对比度
class TextureExpert : public FeatureExtractor {
public:
    vector<float> extract(const Mat& img) override {
        Mat gray, glcm(256, 256, CV_32F, Scalar(0));
        cvtColor(img, gray, COLOR_BGR2GRAY);

        // 生成灰度共生矩阵（水平方向）
        for(int y=0; y<gray.rows; ++y) {
            for(int x=0; x<gray.cols-1; ++x) {
                int i = gray.at<uchar>(y, x);
                int j = gray.at<uchar>(y, x+1);
                glcm.at<float>(i, j) += 1.0;
            }
        }
        normalize(glcm, glcm, 1, 0, NORM_L1);

        // 计算对比度
        float contrast = 0;
        for(int i=0; i<256; ++i) {
            for(int j=0; j<256; ++j) {
                contrast += (i-j)*(i-j) * glcm.at<float>(i,j);
            }
        }
        return {contrast};
    }
};

// 形状专家：Hu矩
class ShapeExpert : public FeatureExtractor {
public:
    vector<float> extract(const Mat& img) override {
        Mat gray, edges;
        vector<vector<Point>> contours;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        Canny(gray, edges, 100, 200);
        findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // 获取最大轮廓
        auto maxContour = *max_element(contours.begin(), contours.end(),
            [](auto& a, auto& b) { return contourArea(a) < contourArea(b); });

        // 计算Hu矩
        Moments m = moments(maxContour);
        double hu[7];
        HuMoments(m, hu);

        // 对数变换增强稳定性
        vector<float> features;
        for(int i=0; i<7; ++i) {
            features.push_back(-copysign(1.0, hu[i]) * log10(abs(hu[i])));
        }
        return features;
    }
};

// 分类决策系统
class Classifier {
    vector<FeatureExtractor*> experts;
    vector<vector<vector<float>>> trainFeatures; // [专家][样本][特征]
    vector<int> trainLabels;
    vector<float> weights = {0.4, 0.3, 0.3}; // 颜色/纹理/形状权重

public:
    Classifier() {
        experts.push_back(new ColorExpert());
        experts.push_back(new TextureExpert());
        experts.push_back(new ShapeExpert());
    }

    void train(const vector<Mat>& samples, const vector<int>& labels) {
        trainFeatures.resize(experts.size());
        for(size_t i=0; i<samples.size(); ++i) {
            for(size_t e=0; e<experts.size(); ++e) {
                trainFeatures[e].push_back(experts[e]->extract(samples[i]));
            }
        }
        trainLabels = labels;
    }

    int predict(const Mat& img) {
        vector<vector<float>> testFeatures;
        for(auto& e : experts) {
            testFeatures.push_back(e->extract(img));
        }

        // 计算相似度得分
        vector<float> scores(trainLabels.size(), 0);
        for(size_t s=0; s<trainLabels.size(); ++s) {
            for(size_t e=0; e<experts.size(); ++e) {
                float dist = chiSquaredDistance(testFeatures[e], trainFeatures[e][s]);
                scores[s] += weights[e] * (1 - dist);
            }
        }

        // 返回最高分对应的标签
        return trainLabels[max_element(scores.begin(), scores.end()) - scores.begin()];
    }

private:
    // 卡方距离比较直方图
    float chiSquaredDistance(const vector<float>& a, const vector<float>& b) {
        float sum = 0;
        for(size_t i=0; i<a.size(); ++i) {
            if(a[i] + b[i] > 1e-6) {
                sum += (a[i]-b[i])*(a[i]-b[i]) / (a[i]+b[i]);
            }
        }
        return sum/2;
    }
};
}
#endif

/**
假设需要分类工业零件：
- 红色圆形垫片（颜色直方图偏红，低纹理对比度，Hu矩接近圆）
- 蓝色方形螺母（颜色直方图偏蓝，高纹理对比度，Hu矩接近矩形）

当输入测试图像时：
1. 颜色专家判断主色调
2. 纹理专家分析表面加工痕迹
3. 形状专家识别几何外形
4. 综合加权得分确定最终类别
 */
void main_MOE_algo2() {
#ifdef OpenCV_DIR
    using namespace moe2;
    String dir = "/home/heaven7/Pictures/test_imgs/";
    // 训练数据准备（实际应来自文件）
    vector<Mat> trainImgs = {
        imread(dir + "red_circle.png"),
        imread(dir + "blue_square.png"),
        // 添加更多训练样本...
    };
    vector<int> labels = {0, 1}; // 0:圆形，1:方形

    // 训练分类器
    Classifier clf;
    clf.train(trainImgs, labels);

    // 测试图像
    Mat testImg = imread(dir + "1.png");
    int result = clf.predict(testImg);

    cout << "Classification result: "
         << (result == 0 ? "Circle" : "Square") << endl;
#endif
}
