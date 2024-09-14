#include "./recorder_test.h"

struct RecorderTestData {
	Recording recording;
	float time;
	PropertyIndexs expectedValue;
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


std::vector<RecorderTestData> recordingTests {
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 0.f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 0,
			.highIndex = 1,
			.percentage = 0.f,
			.complete = false,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
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
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 7.5f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 1,
			.highIndex = 2,
			.percentage = 0.5f,
			.complete = false,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 10.5f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 1,
			.highIndex = 1,
			.percentage = 1.f,
			.complete = true,
		},
	},
};
void recorderIndexsForRecordingTest(){
	for (auto &recordingTest : recordingTests){
		auto indexs = indexsForRecording(recordingTest.recording, recordingTest.time);
		if (!equalIndexs(indexs, recordingTest.expectedValue)){
    	throw std::logic_error(std::string("Invalid index test wanted: ") + indexToStr(recordingTest.expectedValue) + " got: " + indexToStr(indexs));
		}
  } 
}

std::vector<RecorderTestData> reverseRecordingTests {
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 0.f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 2,
			.highIndex = 2,
			.percentage = 1.f,
			.complete = false,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 2.5f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 1,
			.highIndex = 2,
			.percentage = 0.5f,
			.complete = false,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 7.5f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 0,
			.highIndex = 1,
			.percentage = 0.5f,
			.complete = false,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 3.f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 0,
			.highIndex = 1,
			.percentage = 0.7f,
			.complete = false,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 10.5f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 0,
			.highIndex = 0,
			.percentage = 1.f,
			.complete = true,
		},
	},
	RecorderTestData {
		.recording = Recording {
			.keyframes = {
				Record { .time = 0, .properties = {} },
				Record { .time = 5, .properties = {} },
				Record { .time = 10, .properties = {} },
			},
		},
		.time = 5.f,
		.expectedValue = PropertyIndexs {
			.lowIndex = 1,
			.highIndex = 2,
			.percentage = 0.f,
			.complete = false,
		},
	},
};
void recorderIndexsForRecordingReverseTest(){
	for (auto &recordingTest : reverseRecordingTests){
		auto indexs = indexsForRecordingReverse(recordingTest.recording, recordingTest.time);
		if (!equalIndexs(indexs, recordingTest.expectedValue)){
    	throw std::logic_error(std::string("Invalid index test wanted: ") + indexToStr(recordingTest.expectedValue) + " got: " + indexToStr(indexs));
		}
  } 
}
