/** @file matcher.h
 *
 * \brief The core of matching Visual Odometry
 * \author Michal Nowicki
 *
 */

#ifndef _MATCHER_H_
#define _MATCHER_H_

#include "../Defs/putslam_defs.h"
#include <string>
#include <vector>
#include "opencv/cv.h"
#include "../../3rdParty/tinyXML/tinyxml2.h"
#include "../TransformEst/RANSAC.h"

namespace putslam {
/// Grabber interface
class Matcher {
public:
	struct featureSet {
		std::vector<cv::KeyPoint> feature2D;
		cv::Mat descriptors;
		std::vector<Eigen::Vector3f> feature3D;
	};

	struct parameters {
		std::string detector;
		std::string descriptor;
	};

	/// Overloaded constructor
	Matcher(const std::string _name) :
			name(_name), frame_id(0) {

		// TODO: LOAD IT FROM FILE !!!
		float distortionCoeffs[5] = { -0.0410, 0.3286, 0.0087, 0.0051, -0.5643 };
		float cameraMatrix[3][3] = { { 517.3, 0, 318.6 }, { 0, 516.5, 255.3 }, { 0,
						0, 1 } };
		cameraMatrixMat = cv::Mat(3, 3, CV_32FC1, &cameraMatrix);
		distortionCoeffsMat = cv::Mat(1, 5, CV_32FC1, &distortionCoeffs);
	}
	Matcher(const std::string _name, const std::string parametersFile) :
			name(_name), frame_id(0), matcherParameters(parametersFile) {
		// TODO: LOAD IT FROM FILE !!!
		float distortionCoeffs[5] = { -0.0410, 0.3286, 0.0087, 0.0051, -0.5643 };
		float cameraMatrix[3][3] = { { 517.3, 0, 318.6 }, { 0, 516.5, 255.3 }, {
				0, 0, 1 } };
		cameraMatrixMat = cv::Mat(3, 3, CV_32FC1, &cameraMatrix);
		distortionCoeffsMat = cv::Mat(1, 5, CV_32FC1, &distortionCoeffs);
	}

	~Matcher() {
	}

	/// Name of the Matcher
	virtual const std::string& getName() const = 0;

	/// Load features at the start of the sequence
	void loadInitFeatures(const SensorFrame& sensorData);

	/// Get current set of features
	Matcher::featureSet getFeatures();

	/// Run single match
	bool match(const SensorFrame& sensorData,
			Eigen::Matrix4f &estimatedTransformation);

	/// Run the match with map
	bool match(std::vector<MapFeature> mapFeatures,
			std::vector<MapFeature> &foundInlierMapFeatures);

	/// Class used to hold all parameters
	class Parameters {
	public:
		Parameters() {
		}
		;
		Parameters(std::string configFilename) {
			tinyxml2::XMLDocument config;
			std::string filename = "../../resources/" + configFilename;
			config.LoadFile(filename.c_str());
			if (config.ErrorID()) {
				std::cout << "Unable to load Matcher OpenCV config file: "
						<< configFilename << std::endl;
			}
			tinyxml2::XMLElement * params = config.FirstChildElement("Matcher");
			// Matcher
			params->QueryIntAttribute("verbose", &verbose);
			// RANSAC
			params->FirstChildElement("RANSAC")->QueryIntAttribute("verbose",
					&RANSACParams.verbose);
			params->FirstChildElement("RANSAC")->QueryDoubleAttribute(
					"inlierThreshold", &RANSACParams.inlierThreshold);
			params->FirstChildElement("RANSAC")->QueryDoubleAttribute(
					"minimalInlierRatioThreshold",
					&RANSACParams.minimalInlierRatioThreshold);
			params->FirstChildElement("RANSAC")->QueryIntAttribute("usedPairs",
					&RANSACParams.usedPairs);
			// Matcher OpenCV
			OpenCVParams.detector =
					params->FirstChildElement("MatcherOpenCV")->Attribute(
							"detector");
			OpenCVParams.descriptor =
					params->FirstChildElement("MatcherOpenCV")->Attribute(
							"descriptor");

		}
	public:
		int verbose;
		RANSAC::parameters RANSACParams;
		Matcher::parameters OpenCVParams;
	};

protected:

	/// Matcher name
	const std::string name;

	/// Frame id
	uint_fast32_t frame_id;

	/// Information about previous keypoints + descriptors
	std::vector<cv::KeyPoint> prevFeatures;
	cv::Mat prevDescriptors;
	std::vector<Eigen::Vector3f> prevFeatures3D;
	cv::Mat prevRgbImage, prevDepthImage;

	/// Parameters
	Parameters matcherParameters;

	/// Methods used to visualize results/data
	void showFeatures(cv::Mat rgbImage, std::vector<cv::KeyPoint> features);
	void showMatches(cv::Mat prevRgbImage,
			std::vector<cv::KeyPoint> prevFeatures, cv::Mat rgbImage,
			std::vector<cv::KeyPoint> features,
			std::vector<cv::DMatch> matches);

	/// Detect features
	virtual std::vector<cv::KeyPoint> detectFeatures(cv::Mat rgbImage) = 0;

	/// Describe features
	virtual cv::Mat describeFeatures(cv::Mat rgbImage,
			std::vector<cv::KeyPoint> features) = 0;

	/// Perform matching
	virtual std::vector<cv::DMatch> performMatching(cv::Mat prevDescriptors,
			cv::Mat descriptors) = 0;

private:
	cv::Mat cameraMatrixMat;
	cv::Mat distortionCoeffsMat;

	cv::Mat extractMapDescriptors(std::vector<MapFeature> mapFeatures);
	std::vector<Eigen::Vector3f> extractMapFeaturesPositions(std::vector<MapFeature> mapFeatures);
};
}
;

#endif // _MATCHER_H_
