#pragma once
#include "engine.h"
#include <fstream>

using namespace std::chrono;

// Utility method for checking if a file exists on disk
inline bool doesFileExist (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}


struct poly {
    float x, y;

};

struct Object {
    // The object class.
    int label{};
    // The detection's confidence probability.
    float probability{};
    // The object bounding box rectangle.
    cv::Rect_<float> rect;
    // Semantic segmentation mask
    cv::Mat boxMask;
    // Pose estimation key points
    std::vector<float> kps{};


    // 폭력 감지를 위한 관절 각
    float LkickDeg;
    float RkickDeg;

    float LArmLineAngle; //< L 손목-팔목 라인 각도
    float RArmLineAngle; //< R 손목-팔목 라인 각도

    float LLegLineAngle; //< L 정강이 라인 각도
    float RLegLineAngle; //< R 정강이 라인 각도

    float LPunchAngle;
    float RPunchAngle;


    poly LPunchX1;
    poly LPunchX2;
    poly LPunchX3;

    poly RPunchX1;
    poly RPunchX2;
    poly RPunchX3;

    poly LKickX1;
    poly LKickX2;
    poly LKickX3;

    poly RKickX1;
    poly RKickX2;
    poly RKickX3;
};


enum EVENT_TYPE {
    EVENT_NONE = 0,
    EVENT_KNIFE = 100,
    EVENT_COLLAPSE = 101,
    EVENT_VIOLENCE_PUNCH = 102,
    EVENT_VIOLENCE_KICK = 103,
};


// Config the behavior of the YoloV8 detector.
// Can pass these arguments as command line parameters.
struct YoloV8Config {
    // The precision to be used for inference
    Precision precision = Precision::FP16;
    // Calibration data directory. Must be specified when using INT8 precision.
    std::string calibrationDataDirectory;
    // Probability threshold used to filter detected objects
    float probabilityThreshold = 0.70f;
    // Non-maximum suppression threshold
    float nmsThreshold = 0.45f;
    // Max number of detected objects to return
    int topK = 100;
    // Segmentation config options
    int segChannels = 32;
    int segH = 160;
    int segW = 160;
    float segmentationThreshold = 0.5f;
    float wheelchairThreshold = 0.70f;
    // Pose estimation options
    int numKPS = 17;
    float kpsThreshold = 0.5f;
    
    // Class thresholds (default are COCO classes) + wheelchair
    std::vector<std::string> classNames = {
       "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
       "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
       "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
       "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
       "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
       "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
       "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
       "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
       "hair drier", "toothbrush" , "wheelchair"
    };

};

class YoloV8 {
public:
    // Builds the onnx model into a TensorRT engine, and loads the engine into memory
    YoloV8(const std::string& onnxModelPath, const YoloV8Config& config, HWND hParent = NULL);

    // Detect the objects in the image
    std::vector<Object> detectObjects(const cv::Mat& inputImageBGR);
    std::vector<Object> detectObjects(const cv::cuda::GpuMat& inputImageBGR);

    // Draw the object bounding boxes and labels on the image
    void drawObjectLabels(cv::Mat& image, std::vector<Object> &objects, unsigned int scale = 2);
private:
    // Preprocess the input
    std::vector<std::vector<cv::cuda::GpuMat>> preprocess(const cv::cuda::GpuMat& gpuImg);

    // Postprocess the output
    std::vector<Object> postprocessDetect(std::vector<float>& featureVector);

    // Postprocess the output for pose model
    std::vector<Object> postprocessPose(std::vector<float>& featureVector);

    // Postprocess the output for segmentation model
    std::vector<Object> postProcessSegmentation(std::vector<std::vector<float>>& featureVectors);

    std::unique_ptr<Engine> m_trtEngine = nullptr;


#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

    float get_deg(float x1, float y1, float x2, float y2) {
        if (x1 == 0 || x2 == 0)
            return 0.0;
        double dx = x2 - x1;
        double dy = y2 - y1;
        double rad = atan2(dy, dx);
        double degree = (rad * 180) / M_PI;
        if (degree < 0)
            degree += 360;
        return degree;
    }



    double get_angle(poly& a, poly& b, poly& c) {
        double aa, bb, cc;
        double ang, temp;

        aa = sqrt(pow(a.x - c.x, 2) + pow(a.y - c.y, 2));
        bb = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
        cc = sqrt(pow(b.x - c.x, 2) + pow(b.y - c.y, 2));

        temp = (pow(bb, 2) + pow(cc, 2) - pow(aa, 2)) / (2 * bb * cc);
        ang = acos(temp);
        ang = ang * (180 / M_PI);

        return ang;
    }



    float get_dist(float x1, float y1, float x2, float y2) {
        if (x1 == 0 || x2 == 0)
            return 0.0;
        else {
            return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
        }
    }


    // Used for image preprocessing
    // YoloV8 model expects values between [0.f, 1.f] so we use the following params
    const std::array<float, 3> SUB_VALS {0.f, 0.f, 0.f};
    const std::array<float, 3> DIV_VALS {1.f, 1.f, 1.f};
    const bool NORMALIZE = true;

    HWND m_hDlg = NULL;

    float m_ratio = 1;
    float m_imgWidth = 0;
    float m_imgHeight = 0;
    int m_nCollapseCnt = 0;
    int m_nNormalCnt = 0;

    // Filter thresholds
    const float PROBABILITY_THRESHOLD;
    const float NMS_THRESHOLD;
    const int TOP_K;

    // Segmentation constants
    const int SEG_CHANNELS;
    const int SEG_H;
    const int SEG_W;
    const float SEGMENTATION_THRESHOLD;

    // Object classes as strings
    const std::vector<std::string> CLASS_NAMES;

    // Pose estimation constant
    const int NUM_KPS;
    const float KPS_THRESHOLD;

    // Color list for drawing objects
    const std::vector<std::vector<float>> COLOR_LIST = {
            {1, 1, 1},
            {0.098, 0.325, 0.850},
            {0.125, 0.694, 0.929},
            {0.556, 0.184, 0.494},
            {0.188, 0.674, 0.466},
            {0.933, 0.745, 0.301},
            {0.184, 0.078, 0.635},
            {0.300, 0.300, 0.300},
            {0.600, 0.600, 0.600},
            {0.000, 0.000, 1.000},
            {0.000, 0.500, 1.000},
            {0.000, 0.749, 0.749},
            {0.000, 1.000, 0.000},
            {1.000, 0.000, 0.000},
            {1.000, 0.000, 0.667},
            {0.000, 0.333, 0.333},
            {0.000, 0.667, 0.333},
            {0.000, 1.000, 0.333},
            {0.000, 0.333, 0.667},
            {0.000, 0.667, 0.667},
            {0.000, 1.000, 0.667},
            {0.000, 0.333, 1.000},
            {0.000, 0.667, 1.000},
            {0.000, 1.000, 1.000},
            {0.500, 0.333, 0.000},
            {0.500, 0.667, 0.000},
            {0.500, 1.000, 0.000},
            {0.500, 0.000, 0.333},
            {0.500, 0.333, 0.333},
            {0.500, 0.667, 0.333},
            {0.500, 1.000, 0.333},
            {0.500, 0.000, 0.667},
            {0.500, 0.333, 0.667},
            {0.500, 0.667, 0.667},
            {0.500, 1.000, 0.667},
            {0.500, 0.000, 1.000},
            {0.500, 0.333, 1.000},
            {0.500, 0.667, 1.000},
            {0.500, 1.000, 1.000},
            {1.000, 0.333, 0.000},
            {1.000, 0.667, 0.000},
            {1.000, 1.000, 0.000},
            {1.000, 0.000, 0.333},
            {1.000, 0.333, 0.333},
            {1.000, 0.667, 0.333},
            {1.000, 1.000, 0.333},
            {1.000, 0.000, 0.667},
            {1.000, 0.333, 0.667},
            {1.000, 0.667, 0.667},
            {1.000, 1.000, 0.667},
            {1.000, 0.000, 1.000},
            {1.000, 0.333, 1.000},
            {1.000, 0.667, 1.000},
            {0.000, 0.000, 0.333},
            {0.000, 0.000, 0.500},
            {0.000, 0.000, 0.667},
            {0.000, 0.000, 0.833},
            {0.000, 0.000, 1.000},
            {0.000, 0.167, 0.000},
            {0.000, 0.333, 0.000},
            {0.000, 0.500, 0.000},
            {0.000, 0.667, 0.000},
            {0.000, 0.833, 0.000},
            {0.000, 1.000, 0.000},
            {0.167, 0.000, 0.000},
            {0.333, 0.000, 0.000},
            {0.500, 0.000, 0.000},
            {0.667, 0.000, 0.000},
            {0.833, 0.000, 0.000},
            {1.000, 0.000, 0.000},
            {0.000, 0.000, 0.000},
            {0.143, 0.143, 0.143},
            {0.286, 0.286, 0.286},
            {0.429, 0.429, 0.429},
            {0.571, 0.571, 0.571},
            {0.714, 0.714, 0.714},
            {0.857, 0.857, 0.857},
            {0.741, 0.447, 0.000},
            {0.741, 0.717, 0.314},
            {0.000, 0.500, 0.500},
            {1.000, 0.667, 1.000},  //< Wheelchair
    };

    const std::vector<std::vector<unsigned int>> KPS_COLORS = {
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {51, 153, 255},
            {51, 153, 255},
            {51, 153, 255},
            {51, 153, 255},
            {51, 153, 255},
            {51, 153, 255}
    };

    const std::vector<std::vector<unsigned int>> SKELETON = {
            {16, 14},
            {14, 12},
            {17, 15},
            {15, 13},
            {12, 13},
            {6, 12},
            {7, 13},
            {6, 7},
            {6, 8},
            {7, 9},
            {8, 10},
            {9, 11},
            {2, 3},
            {1, 2},
            {1, 3},
            {2, 4},
            {3, 5},
            {4, 6},
            {5, 7}
    };

    const std::vector<std::vector<unsigned int>> LIMB_COLORS = {
            {51, 153, 255},
            {51, 153, 255},
            {51, 153, 255},
            {51, 153, 255},
            {255, 51, 255},
            {255, 51, 255},
            {255, 51, 255},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {255, 128, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 255, 0}
    };
};