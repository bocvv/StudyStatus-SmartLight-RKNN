#include <string>
#include <iostream>

#include "study_status.h"


template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}

int main(int argc, char* argv[]) {
    /*
     * I can not use VideoCapture on Android system!!!
    int cam_index = 0;
    int cam_width = 3840;
    int cam_height = 2160;

    if (argc >= 2) {
        cam_index = atoi(argv[1]);
    }

    cv::VideoCapture cap;
    cap.open("./VID_20210526_145858.mp4");
    if (!cap.isOpened()) {
        std::cout << "video not open." << std::endl;
        return 0;
    }

    cap.set(cv::CAP_PROP_AUTOFOCUS, 0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, cam_width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, cam_height);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    */

    // init StudyStatus obj
    StudyStatusManager *ss_mgr = new StudyStatusManager();
    int r = ss_mgr->init("./data/study_status.cfg");

    cv::Mat g_frame;
    std::string prefix = "./frames/";
    int idx{1};
    while (1) {
        if (idx > 1000) {
            break;
        }
        std::string img_path = prefix + to_string(idx++) + ".jpg";
        g_frame = cv::imread(img_path, 1);
        cv::cvtColor(g_frame, g_frame, cv::COLOR_BGR2RGB);
        StudyStatusResult res = ss_mgr->get_study_status(g_frame, 1920, 1080);
        std::cout << "frame_id = " << idx << "\n" <<
                     "back pose: [" << int(res.back) << "], score = " << res.back_score << "\n" << 
                     "head pose: [" << int(res.head) << "], score = " << res.head_score << "\n" <<  
                     "action pose: [" << int(res.action) << "], score = " << res.action_score << "\n" << std::endl;
    }

    // release StudyStatus obj
    ss_mgr->release();
    delete ss_mgr;

    return 0;
}
