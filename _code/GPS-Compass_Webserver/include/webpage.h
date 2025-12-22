#pragma once
#include <pgmspace.h>

/* ===================== EMBEDDED WEBPAGE ===================== */
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>GPS-Compass Webserver</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body style="font-family: Comic Sans MS;"> 
  <h1> ENTER POSITION:</h1>

  <div style="display: inline-grid; grid-template-columns: auto auto; font-family: Verdana; font-size: 150%;">
    <div style="display: inline-grid; grid-template-columns: auto auto;">
      <div>
        <input type="number" style="width: 200px;" value="0" id="latBox" min="0"
          oninput="sliderFunc('latBox', 'latSlider')"> <br>
        <input type="range" style="width: 200px;" value="0" id="latSlider" step="0.01" min="0" max="180"
          oninput="sliderFunc('latSlider', 'latBox')">
      </div>
      <div style="display: inline-grid; grid-template-columns: auto auto;">
        <select name="lat" id="lat">
          <option value="n">N</option>
          <option value="s">S</option>
        </select>
      </div>
    </div>
    <div style="display: inline-grid; grid-template-columns: auto auto;">
      <div>
        <input type="number" style="width: 200px;" value="0" id="lonBox" min="0"
          oninput="sliderFunc('lonBox', 'lonSlider')"> <br>
        <input type="range" style="width: 200px;" value="0" id="lonSlider" step="0.001" min="0" max="180"
          oninput="sliderFunc('lonSlider', 'lonBox')">
      </div>
        <div style="display: inline-grid; grid-template-columns: auto auto;">
          <select name="lat" id="lat">
          <option value="e">E</option>
          <option value="w">W</option>
        </select>
      </div>
    </div>
  </div>
  <div>
  	<input style="font-family: Comic Sans MS;" type="button" value="UPLOAD COORDINATES" onclick="upload_coords()">
    </div>
  <h2 style="font-family: Comic Sans MS;" >LED Control (GPIO 2)</h2>
  <input style="font-family: Comic Sans MS;" type="button" value="LED ON" onclick="fetch('/led?state=1')">
  <input style="font-family: Comic Sans MS;" type="button" value="LED OFF" onclick="fetch('/led?state=0')">


<script>
    function sliderFunc(inputIn, inputOut) {
      var val = document.getElementById(inputIn).value
      //document.getElementById(inputOut).value = Math.round((Math.abs(val) * scale)).toString()
      document.getElementById(inputOut).value = Math.abs(val).toFixed(5)
    }
    function uload_coords() {
    	var lat_string = document.getElementById(latBox).valuetoFixed(4)

    }
  </script>
</body>
</html>
)rawliteral";
/* ============================================================ */
