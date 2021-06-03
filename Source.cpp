#include<iostream>
#include<opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<vector>

using namespace std;
using namespace cv;
/// <summary>
/// Test rotation.
/// </summary>
/// <param name="img">Source image (grayscale)</param>
/// <param name="dest">Output image</param>
/// <param name="r">Minimum area rectangle of the largest contours </param>
void rotateTest(const Mat& img, Mat& dest, const RotatedRect r) {
	Point center = Point(r.center.x, r.center.y);

	double angle = (r.size.width > r.size.height) ? 90 + r.angle : r.angle;

	Mat rotMat = getRotationMatrix2D(Point2f(center.x, center.y), angle, 1);
	warpAffine(img, dest, rotMat, img.size(), INTER_CUBIC);
}


/// <summary>
/// 
/// </summary>
/// <param name="img"></param>
/// <param name="dest"></param>
/// <param name="newSize"></param>
void standardize(const Mat& img, Mat& dest) {
	//threshold the image to get the lines (threshold value: 130, invert the result)
	Mat tmp;
	threshold(img, tmp, 130, 255, THRESH_BINARY_INV);

	//use median blur with size 3
	medianBlur(tmp, tmp, 3);

	vector<vector<Point>> contours;
	//find the external (RETR_EXTERNAL) contours, do NOT use hierarchy
	findContours(tmp, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	//use the minAreaRect function to get a RotatedRect around the 0th contour:
	RotatedRect r = minAreaRect(contours[0]);

	//rotate the original image with the rotateTest function
	//rotete the prepared image wi	th the rotateTest function
	Mat img2, tmp2;
	rotateTest(img, img2, r);
	rotateTest(tmp, tmp2, r);


	//find the external (RETR_EXTERNAL) contours again, do NOT use hierarchy
		//findContours(img2, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	findContours(tmp2, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	// we crop the rotated image based on the bounding rect.
	dest = img2(boundingRect(contours[0]));
}

void createTheMaskOfTheNonGrayCells(const Mat& solver, Mat& dest) {

	Mat gray_cells;

	// use an inRange method to select the gray cells on the image (between 100 and 200)
	inRange(solver, 100, 200, gray_cells);


	// use a median blur with the size 5
	medianBlur(gray_cells, gray_cells, 5);
	// use an erode operator with this structure elements:
	Mat struct_element = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
	erode(gray_cells, gray_cells, struct_element);
	// invert the result and save into the dest mat
	dest = 255 - gray_cells;
}

int main() {

	Mat solver0 = imread("test_solver.png", IMREAD_GRAYSCALE);

	Mat solver;
	standardize(solver0, solver);

	Mat nongray;
	createTheMaskOfTheNonGrayCells(solver, nongray);


	for (int i = 0; i <= 17; ++i) {
		Mat img0 = imread("test_" + to_string(i) + ".png", IMREAD_GRAYSCALE);

		Mat img, tmp;
		standardize(img0, img);

		//resize the image to the size of the nongray image, use INTER_NEAREST method
		resize(img, tmp, nongray.size(), INTER_NEAREST);


		//threshold the result img to get the X-es and the lines
		// value: 150, invert the result
		Mat thimg;
		threshold(tmp, thimg, 150, 250, THRESH_BINARY_INV);
		// this step will keep the correct answers (X) on the image and remove everything else
		thimg.setTo(0, nongray);


		// if you have something else, you should fix the previsous parts.
		imshow("correct answers", thimg);
		imshow("solver", solver);
		waitKey(0);

		//maybe some X has more than one part, so this merge those parts together 
		dilate(thimg, thimg, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


		vector<vector<Point>> contours;
		// find the external contour to get the contour of X-es
		findContours(thimg, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		// to visualize the result
		Mat canvas;
		cvtColor(img, canvas, COLOR_GRAY2BGR);

		// iterate over the contours
		for (auto c : contours) {
			// find the bounding box of c
			Rect r = boundingRect(c);

			// draw the rectangele onto the canvas
			rectangle(canvas, r, Scalar(0, 255, 0));

			imshow("original", img);

			imshow("answer", canvas);
			//get the coordinates of the cells
			// 62x42 is the size of the empty rectangle of the top left corner
			// 48x43 is the size of the cells
			// because of the standardization part, each test image has the same size
			char columns = (char)((r.x - 62) / 48 + 'A');
			int rows = (r.y - 43) / 42 + 1;
			cout << columns << rows << endl;

			waitKey(0);
		}

	}



	return 0;
}