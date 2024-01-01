#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

// Generate a view matrix, receving eye postion as parameter
// The position of the camera $e$, we should move the e -> (0, 0, 0)
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    // initializaiton the 4 \times 4 E
    // 1    0   0   0
    // 0    1   0   0
    // 0    0   1   0
    // 0    0   0   1
    // Construct the translation matrix Set the view matrix 
    // to a matrix with the origin at the given eye position
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0,  1;

    view = translate * view; // the view change the origin to the eye position

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    // [ cos(theta) -sin(theta) 0 0 ]
    // [ sin(theta) cos(theta)  0 0 ]
    // [     0           0      1 0 ]
    // [     0           0      0 1 ]

    float angle_rad = rotation_angle * MY_PI / 180.0f; // deg -> rad
    float cos_theta = std::cos(angle_rad);
    float sin_theta = std::sin(angle_rad);

    model << cos_theta, -sin_theta,    0, 0,
                sin_theta, cos_theta,  0, 0,
                0,         0,          1, 0,
                0,         0,          0, 1;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    
    // Students will implement this function
    
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    Eigen::Matrix4f proj, ortho;

    proj << zNear, 0, 0, 0,
            0, zNear, 0, 0,
            0, 0, zNear + zFar, -zNear * zFar,
            0, 0, 1, 0;

    double w, h, z;
    h = zNear * tan(eye_fov / 2) * 2;
    w = h * aspect_ratio;
    z = zFar - zNear;

    ortho << 2 / w, 0, 0, 0,
             0, 2 / h, 0, 0,
             0, 0, 2 / z, -(zFar+zNear) / 2,
             0, 0, 0, 1;   // because the center is (x, y) already, so it is not to move (x, y)
             				
    projection = ortho * proj * projection;
    
    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {//任意轴旋转矩阵（罗德里格斯旋转公式，默认轴过原点）
    double fangle = angle / 180 * MY_PI;
    Eigen::Matrix4f I, N, Rod;
    Eigen::Vector4f axi;
    Eigen::RowVector4f taxi;

    axi << axis.x(), axis.y(), axis.z(), 0;

    I.Identity();

    N << 0, -axis.z(), axis.y(), 0,
         axis.z(), 0, -axis.x(), 0,
         -axis.y(), axis.x(), 0, 0,
         0, 0, 0, 1;
    
    Rod = cos(fangle) * I + (1 - cos(fangle)) * axi * axi.transpose() + sin(fangle) * N;
    Rod(3, 3) = 1;//这里要注意，非齐次坐标的公式应用在齐次坐标上时记得运算完成后把矩阵的右下角改为1，否则会导致图形比例错误
    return Rod;
}


int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700); // create the object of rasterizer

    Eigen::Vector3f eye_pos = {0, 0, 5}; // the position of eye

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}}; // the Triangle position in 3D

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}}; // the index of triangle

    auto pos_id = r.load_positions(pos); // load 
    auto ind_id = r.load_indices(ind);

    int key = 0; // the input of keyboard
    int frame_count = 0; // time

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth); // clear color and buffer before draw

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        

        // Convert framebuffer to image and save
        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) { // enter ESC to exit
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        // Convert framebuffer to image and display
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10); // wait the keyboard enter, every 10ms check out

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10; // enter 'a' clockwise rotation
        }
        else if (key == 'd') {
            angle -= 10; // enter 'd' rotate counterclockwise
        }
    }

    return 0;
}
