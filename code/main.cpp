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


int start_video_capture(Neck *neck, float c, int d){
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
  threshold_coefficient = c;

  top_threshold = (int)(camera_width - threshold_coefficient * camera_width);
  bottom_threshold = (int)(threshold_coefficient * camera_width);



  printf("Creating background subtractor\n");
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
        neck->turn_left(d,40);
      }
      if(centroid.x < bottom_threshold){
        printf("turning right\n");
        neck->turn_right(d,40);
      }
      imshow("Mask", fgMaskMOG2);
      //if(waitKey(30) >= 0) break;
    }
  return 0;
}

int invalid_usage(){
  printf("(╯°□°)╯︵ ┻━┻   INVALID USAGE!!!\n");
  printf("Usage: ./DisplayImage <arduino_port>\n Options:\n -c threshold coefficient (range: 0 - 0.5, default is .3125)\n -d degrees per turn (range: 5-40, default is 20)\n");
  return 0;
}

int main(int argc, char** argv )
{
  int d,i, temp_d;
  float c, temp_c;

  //Defaults
  i=0;
  c=.3125;
  d=20;

  if(argc == 1){
    return invalid_usage();
  }
  if((argc > 2 && argc % 2 == 1) || argc > 6){
       return invalid_usage();
  }
  if(argc > 2){
    std::string::size_type sz;
    for(i = 0; i<((argc-2)/2); i++){
      char* ch = (argv[2+(i*2)]);
      if(!strcmp("-d", ch)){
        temp_d = (int)stof(argv[3+(i*2)]);
        if(temp_d < 5 || temp_d > 40){
          printf("-d is out of range\n");
          return invalid_usage();
        }
        d = temp_d;
      }
      else if(!strcmp("-c", ch)){
        temp_c = stof(argv[3+(i*2)]);
        if(temp_c < 0.1 || temp_c > 0.45){
          printf("-c is out of range\n");
          return invalid_usage();
        }
        c = temp_c;
      }
      else return invalid_usage();
    }
  }
  printf("USING OPENCV Version %d,%d\n",  CV_MAJOR_VERSION, CV_MINOR_VERSION);
  printf("Using rotation increments of %d\n", d);
  printf("Using threshold coefficient of %f\n", c);
  Neck *neck = new Neck(argv[1]);
  printf("Arduino is connected & port is open? %s\n", neck->is_active() ? "true" : "false");
  printf(".");
  fflush(stdout);
  usleep(1000000);
  printf(".");
  fflush(stdout);
  usleep(1000000);
  printf(".");
  fflush(stdout);
  usleep(1000000);
  printf(".");
  fflush(stdout);
  usleep(1000000);
  printf("GO!\n");

  return start_video_capture(neck, c, d);
}

