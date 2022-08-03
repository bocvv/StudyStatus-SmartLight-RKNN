#include <opencv2/opencv.hpp>

#include "base64.h""

using namespace std;

int main(int argc, char* argv[]) {
    /*
    fstream f;
    f.open("10.txt", ios::in);

    if (!f) {
        cout << "open file error!" << endl;
        return -1;
    }

    char line[2000];

    while(!f.eof())
    {
        //使用eof()函数检测文件是否读结束
        f.getline(line, 2000);
        int numBytes = f.gcount();        //使用gcount()获得实际读取的字节数
        cout << line << "\t" << numBytes << "字节" << endl ;
    }

    std::string encoded = "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDACAWGBwYFCAcGhwkIiAmMFA0MCwsMGJGSjpQdGZ6eHJmcG6AkLicgIiuim5woNqirr7EztDOfJri8uDI8LjKzsb/2wBDASIkJDAqMF40NF7GhHCExsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsb/wAARCAYABgADASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwDn6KKKACiiigAooooAKKKKACiiigAooooAKKKKACiiigAooooAKKKKACiiigAooooAKKKKACiiigAooooAUCnDFNFLQBICKcGFRUZpWAmDinbxUG4UbqLATbxRvFQbjRzRYCfeKTeKhpwWjlC5J5gpCxNIBRVJE3CiiimAUlLRQAlFLSU";
    */
    std::string encoded;
    cv::Mat img;
    img = cv::imread("camera1_1626950688799.jpg");
    int r = compress_and_encode(img, encoded);
    cout << encoded << endl;

    cv::Mat decoded;
    r = decode_base64_to_mat(encoded, decoded);
    cv::imwrite("res.jpg", decoded);

    return 0;
}
