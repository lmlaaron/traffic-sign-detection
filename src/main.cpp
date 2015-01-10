// stl library
#include <string>
#include <iostream>

// OpenCV library
#include <opencv2/opencv.hpp>

// our own code
#include "segmentation.h"
#include "colorConversion.h"
#include "imageProcessing.h"

int main(int argc, char *argv[]) {

  // Chec the number of arguments
  if (argc != 2) {
    std::cout << "********************************" << std::endl;
    std::cout << "Usage of the code: ./traffic-sign-detection imageFileName.extension" << std::endl;
    std::cout << "********************************" << std::endl;

    return -1;
  }

  // Read the input image - convert char* to string
  std::string input_filename(argv[1]);

  // Read the input image
  cv::Mat input_image = cv::imread(input_filename);

  // Check that the image has been opened 
  if (!input_image.data) {
    std::cout << "Error to read the image. Check ''cv::imread'' function of OpenCV" << std::endl;
    return -1;
  }
  // Check that the image read is a 3 channels image
  CV_Assert(input_image.channels() == 3);


  /*
   * Conversion of the image in some specific color space
   */

  // Conversion of the rgb image in ihls color space
  cv::Mat ihls_image = colorconversion::convert_rgb_to_ihls(input_image);
  // Conversion from RGB to logarithmic chromatic red and blue
  std::vector< cv::Mat > log_image = colorconversion::rgb_to_log_rb(input_image);

  /*
   * Segmentation of the image using the previous transformation
   */

  // Segmentation of the IHLS and more precisely of the normalised hue channel 
  // ONE PARAMETER TO CONSIDER - COLOR OF THE TRAFFIC SIGN TO DETECT - RED VS BLUE
  int nhs_mode = 0; // nhs_mode == 0 -> red segmentation / nhs_mode == 1 -> blue segmentation
  cv::Mat nhs_image_seg = segmentation::seg_norm_hue(ihls_image, nhs_mode);
  // Segmentation of the log chromatic image
  cv::Mat log_image_seg = segmentation::seg_log_chromatic(log_image);

  /*
   * Merging and filtering of the previous segmentation
   */

  // Merge the results of previous segmentation using an OR operator
  // Pre-allocation of an image by cloning a previous image
  cv::Mat merge_image_seg = nhs_image_seg.clone();
  cv::bitwise_or(nhs_image_seg, log_image_seg, merge_image_seg);
  // Filter the image using median filtering and morpho math
  cv::Mat bin_image = imageprocessing::filter_image(merge_image_seg);

  /*
   * Extract candidates (i.e., contours) and remove inconsistent candidates
   */

  std::vector< std::vector< cv::Point > > contours = imageprocessing::contours_extraction(bin_image);
  
  cv::Mat output_image = cv::Mat::zeros(bin_image.size(), CV_8U);
  cv::Scalar color(255,255,255);
  cv::drawContours(output_image, contours, -1, color, 0, 8);
  cv::namedWindow("Window", CV_WINDOW_AUTOSIZE);
  cv::imshow("Window", output_image);
  cv::waitKey(0);

  return 0;
}