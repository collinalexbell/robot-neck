#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <string>
#include <unistd.h>

using namespace cv;
using namespace std;

class Neck{
private:
  boost::asio::io_service *io;
  boost::asio::serial_port *port;
public:
  Neck(String serial_port){
    io = new boost::asio::io_service;
    port = new boost::asio::serial_port(*io);

    port->open(serial_port);
    port->set_option(boost::asio::serial_port_base::baud_rate(9600));

  };
  /*
  ~Neck(){
    port->close();
    delete port;
    delete io;
  }
  */

  int turn_left(int amount, int speed){
    char str[9];
    char rv_buf[5];
    char c_buf[1];
    bool recieved_result = false;
    int rv;
    int i;
    int cur_pos;
    int write_size;

    write_size = snprintf(str, 9, "0,%d,%d\n", amount, speed);
    printf("string: %s with size: %d\n", str, write_size);
    boost::asio::write(*port, boost::asio::buffer(str, write_size));

    //rv = atoi(rv_buf);
		rv = 90;
    return rv;
  }
  int turn_right(int amount, int speed){
    char str[9];
    char rv_buf[5];
    char c_buf[1];
    bool recieved_result = false;
    int rv;
    int i;
    int cur_pos;
    int write_size;

    write_size = snprintf(str, 9, "1,%d,%d\n", amount, speed);
    printf("string: %s with size: %d\n", str, write_size);
    boost::asio::write(*port, boost::asio::buffer(str, write_size));

    
    rv = 90;
    return rv;
  }

  bool is_active(){
    return port->is_open();
  }
};


Point get_centroid(Mat arr){
  Moments m = moments(arr);
  return Point(m.m10/m.m00, m.m01/m.m00);
}


int start_video_capture(Neck *neck){
  VideoCapture cap(0); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;

  Mat frame;
  Mat fgMaskMOG2;
  Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(3); //MOG2 approach
  for(;;)
    {
      cap >> frame; // get a new frame from camera
			printf("frame_size: %d", frame.size().width);
      std::stringstream ss;
      pMOG2->apply(frame, fgMaskMOG2);
      Point centroid = get_centroid(fgMaskMOG2);
      circle(fgMaskMOG2, centroid, 20, Scalar(255,0,0), 10);
      printf("x: %d\n", centroid.x);
      //1280
      if(centroid.x > 450){
        printf("turning left\n");
        neck->turn_left(10,40);
      }
      if(centroid.x < 200){
        printf("turning right\n");
        neck->turn_right(10,40);
      }
      imshow("Mask", fgMaskMOG2);
     //if(waitKey(30) >= 0) break;
    }
  return 0;
}

int main(int argc, char** argv )
{
    printf("USING OPENCV Version %d,%d\n",  CV_MAJOR_VERSION, CV_MINOR_VERSION);
  printf("Arduino location: %s\n", argv[1]);
  Neck *neck = new Neck(argv[1]);
  printf("Port is open? %s\n", neck->is_active() ? "true" : "false");
  //if(neck->is_active()){
  //usleep(4000000);
  //int pos = neck->turn_left(40,40);
  //printf("pos: %d", pos);
    //usleep(4000000);
    //delete neck;
    //printf("cur_pos:%s\n", neck.turn_left(40,40));
  //}
  return start_video_capture(neck);
}

