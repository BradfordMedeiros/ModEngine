#include "./recorder_test.h"

struct RecorderTestData {
	Recording recording;
	float time;
	PropertyIndexs expectedValue;
};


std::vector<RecorderTestData> recordingTests {
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
			},
		},
		.time = 2.5f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 0,
			.highIndex = 1,
			.percentage = 0.5f,
			.complete = false,
		},
	},
};

bool equalIndexs(PropertyIndexs& propertyIndexs1, PropertyIndexs& propertyIndexs2){
	if (propertyIndexs1.lowIndex != propertyIndexs2.lowIndex){
		return false;
	}
	if (propertyIndexs1.highIndex != propertyIndexs2.highIndex){
		return false;
	}
	if (!aboutEqual(propertyIndexs1.percentage, propertyIndexs2.percentage)){
		return false;
	}
	if (propertyIndexs1.complete != propertyIndexs2.complete){
		return false;
	}
	return true;
}
std::string indexToStr(PropertyIndexs& propertyIndexs){
	std::string value = "";
	value += std::string("lowindex=") + std::to_string(propertyIndexs.lowIndex) + std::string(", ");
	value += std::string("highindex=") + std::to_string(propertyIndexs.highIndex) + std::string(", ");
	value += std::string("percentage=") + std::to_string(propertyIndexs.percentage) + std::string(", ");
	value += std::string("complete=") + print(propertyIndexs.complete) + std::string(", ");
	return value;
}

void recorderIndexsForRecordingTest(){
	for (auto &recordingTest : recordingTests){
		auto indexs = indexsForRecording(recordingTest.recording, recordingTest.time);
		if (!equalIndexs(indexs, recordingTest.expectedValue)){
    	throw std::logic_error(std::string("Invalid index test wanted: ") + indexToStr(recordingTest.expectedValue) + " got: " + indexToStr(indexs));
		}
  } 
}
