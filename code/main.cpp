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

    for(i=0; i<5; i++){
      boost::asio::read(*port, boost::asio::buffer(c_buf, 1));
      if(c_buf[0] == '\n'){
        recieved_result = true;
        break;
      }
      else if(c_buf[0] == '\r'){
        //pass
      }
      else{
        rv_buf[i] = c_buf[0];
      }
    }

    if(!recieved_result){
      printf("Error parsing current position after re-position\n");
      //return -1;
    }
    rv = atoi(rv_buf);
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

    for(i=0; i<5; i++){
      boost::asio::read(*port, boost::asio::buffer(c_buf, 1));
      if(c_buf[0] == '\n'){
        recieved_result = true;
        break;
      }
      else if(c_buf[0] == '\r'){
        //pass
      }
      else{
        rv_buf[i] = c_buf[0];
      }
    }

    if(!recieved_result){
      printf("Error parsing current position after re-position\n");
      //return -1;
    }
    rv = atoi(rv_buf);
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
  int top_threshold, bottom_threshold, camera_width;
  float threshold_coefficient;
  Mat frame;
  Mat fgMaskMOG2;


  VideoCapture cap(0); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;

  //Get a new frame from camera because camera width is needed to calculate thresholds
  cap >> frame;
  camera_width = frame.size().width;

  printf("Camera width: %d\n", frame.size().width);

  /*Set the threshold_coefficient
    This threshold coefficient sets the percentage of screen space dedicated to "movement zones".

    If I have a coefficient of .2, that means that 20% of the left & 20% of the right screen is
    dedicated to movement zones for a total of 40% of movement zone.
  */
  threshold_coefficient = 0.3125;

  top_threshold = (int)(camera_width - threshold_coefficient * camera_width);
  bottom_threshold = (int)(threshold_coefficient * camera_width);



  Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(3); //MOG2 approach
  for(;;)
    {
      cap >> frame; // get a new frame from camera
      std::stringstream ss;
      pMOG2->apply(frame, fgMaskMOG2);
      Point centroid = get_centroid(fgMaskMOG2);
      circle(fgMaskMOG2, centroid, 20, Scalar(255,0,0), 10);
      printf("x: %d\n", centroid.x);
      //1280
      if(centroid.x > top_threshold){
        printf("turning left\n");
        neck->turn_left(20,40);
      }
      if(centroid.x < bottom_threshold){
        printf("turning right\n");
        neck->turn_right(20,40);
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

