#include "./alert.h"

extern CustomApiBindings* mainApi;

const float bufferExpirationTimeMs = 5000;
const int maxBufferSize = 1;


struct AlertMessage {
	std::string message;
	double time;
};
struct Alerts {
	std::deque<AlertMessage> messageBuffer;
};


std::string amountToDraw(std::string& text, double createTime, float rate){
	auto currIndex = static_cast<int>((mainApi -> timeSeconds(true) - createTime) * rate);
	return text.substr(0, currIndex);
}

const int letterSize = 8;
const float letterSizeNdi = letterSize / 1000.f;
const float margin = letterSizeNdi * 3;
const float marginLeft = margin;
const float marginBottom = margin;

void renderAlerts(Alerts& alerts, int yoffset, std::deque<AlertMessage>& buffer){
	for (int i = 0; i < buffer.size(); i++){
		auto message = buffer.at(i);
		auto textToDraw = amountToDraw(message.message, message.time, 100);

		mainApi -> drawText(
			textToDraw, 
			(-1 + marginLeft),
			(-1 + (letterSizeNdi * 0.5) + marginBottom), 
			letterSize, 
			false, 
			std::nullopt, 
			std::nullopt,
			true, 
			std::nullopt, 
			std::nullopt
		);
	}
}

bool isNotExpiredMessage(AlertMessage& message){
	auto currTime = mainApi -> timeSeconds(true);
	auto createTime = message.time;
	auto diff = (currTime - createTime) * 1000;
	return diff < bufferExpirationTimeMs;
}

void filterExpiredMessages(Alerts& alerts){
	std::deque<AlertMessage> newMessageBuffer;
	for (auto &message : alerts.messageBuffer){
		if (isNotExpiredMessage(message)){
			newMessageBuffer.push_back(message);
		}
	}
	alerts.messageBuffer = newMessageBuffer;
}

CScriptBinding cscriptAlertsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/alert", api);
  

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Alerts* alerts = new Alerts;
    alerts -> messageBuffer = {};

    //void (*schedule)(objid id, float delayTimeMs, void* data, std::function<void(void*)> fn);
   	return alerts;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Alerts* alerts = new Alerts;
    delete alerts;
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    Alerts* alerts = static_cast<Alerts*>(data);
    renderAlerts(*alerts, 400, alerts -> messageBuffer);
    filterExpiredMessages(*alerts); // probably shouldn't be done every frame
  };

  binding.onMessage = [](objid scriptId, void* data, std::string& topic, std::any& anyValue) -> void {
    Alerts* alerts = static_cast<Alerts*>(data);
    if (topic == "alert"){
    	std::string* strValue = anycast<std::string>(anyValue);
    	modassert(strValue, "cscript editor - any cast invalid, not attribute value");
    	alerts -> messageBuffer.push_back(AlertMessage {
    		.message = *strValue,
    		.time = mainApi -> timeSeconds(true),
    	});
    	if (alerts -> messageBuffer.size() > maxBufferSize){
    		alerts -> messageBuffer.pop_front();
    	}
    }
  };

  return binding;
}
