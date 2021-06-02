let lightOn = false;
let slider_state = 0;

let btnSend = document.querySelector('button');
btnSend.addEventListener('click',() => {
    lightOn = !lightOn;
    send_update();
    updateButton();
});
var slider = document.getElementById("myRange");
var output = document.getElementById("value");

output.innerHTML = slider.value + "";

slider.oninput = function() {
    output.innerHTML = this.value + " %";
}

old_slider_val = 1
slider.addEventListener("mousemove", function(){
    var x = slider.value;
    var color = 'linear-gradient(90deg, rgb(255, 255, 255) ' + x + '%, rgb(107, 107, 107) ' + x + '%)';
    slider.style.background = color;
    if(old_slider_val != x){
      old_slider_val = x
      console.log(x + "%")
      if(!lightOn){
        slider_state = 1;
        //send_update();
        console.log("slider state:" + slider_state);
      }
    }
});

//document.getElementById("all")
//slider.addEventListener("onmouseup", 
function mouseUp(){
  console.log("mouse up");
  if(slider_state == 1){
    send_update();
    slider_state = 0;
  }
}
//);

function updateButton(){
  if(lightOn){
    btnSend.style.background = "rgb(132, 199, 255)";
    btnSend.style.color = "rgb(255, 255, 255)";
    btnSend.textContent = "auto"
  }
  else{
    btnSend.style.background = "rgb(255, 255, 255)";
    btnSend.style.color = "rgb(107, 107, 107)"
    btnSend.textContent = "manual"
  }
}

function updatePeopleCount(n){
  document.getElementById("people_count").innerHTML = n;
}

//------------------------------------------------------------------ backend
function isJson(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
}

//$(document).ready(function(e) {
    client = new Paho.MQTT.Client("mqtt.netpie.io", 443, "a3a9c30c-5b64-480f-bf31-a2fc3b0aa320");
    var options = {
        useSSL: true,
        userName: "puD65xV8NBZWFxYdVnneitAhYiCdnZfe",
        password: "g6SkUaZ141ZAw5GVH)1KSlGdlIM3$I~2",
        onSuccess:onConnect,
        onFailure:doFail
    }
    client.connect(options);

    function onConnect() {
        //$("#status").text("Connected").removeClass().addClass("connected");
        console.log("connected");
        client.subscribe("@msg/data");
        mqttSend("@msg/led", "GET");
    }

    function doFail(e){
        console.log("failed : " + e);
    }

    client.onMessageArrived = function(message) {
        //console.log("message arrived: " + message.payloadString);
        console.log("message arrived: " + isJson(message.payloadString));
        if(isJson(message.payloadString)){
          data = JSON.parse(message.payloadString);
          console.log("data arrived: " + data["mode_auto"] + " " + data["count"]);
          if(data["mode_auto"] == 1 && !lightOn){
            lightOn = true;
            updateButton();
          }
          else if(data["mode_auto"] == 0 && lightOn){
            lightOn = false;
            updateButton();
          }
          updatePeopleCount(data["count"]);
        }
        /*if (message.payloadString == "LEDON" || message.payloadString == "LEDOFF") {
            //$("#led-on").attr("disabled", (message.payloadString == "LEDON" ? true : false));
            //$("#led-off").attr("disabled", (message.payloadString == "LEDOFF" ? true : false));
            console.log("led: " + message.payloadString)
            if(message.payloadString == "LEDON"){
              lightOn = true;
            }
            else{
              lightOn = false;
            }
            updateButton();
        }*/
    }
    /*
    $("#led-on").click(function(e) {
        mqttSend("@msg/led", "LEDON");
        document.getElementById("status-led").innerHTML = "LED is ON";
    });
        
    $("#led-off").click(function(e) {
        mqttSend("@msg/led", "LEDOFF");
        document.getElementById("status-led").innerHTML = "LED is OFF";
    }*/
//  );
var mqttSend = function(topic, msg) {
    var message = new Paho.MQTT.Message(msg);
    message.destinationName = topic;
    client.send(message);
}

function send_update(){
  if(lightOn){
    mqttSend("@msg/led", "SET 1 " + old_slider_val + ";");
  }
  else{
    mqttSend("@msg/led", "SET 0 " + old_slider_val + ";");
  }
  mqttSend("@msg/led", "GET");
}

//------------------------------------------------------------------ backend
//mqttSend("@msg/led", "LIGHTVALUE:" + x);