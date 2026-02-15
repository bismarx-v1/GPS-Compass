#pragma once
#include <pgmspace.h>

/* ===================== EMBEDDED WEBPAGE ===================== */
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>GPS-Compass Webserver</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body, input, select, textarea, button, p, h1, h2 { font-family: Verdana, sans-serif }
  </style>
</head>

<body> 
  <h1 style="margin-bottom: 40px;">GPS-Compass Webserver</h1>
  
  <div id="modeContainer" style="position:sticky; top:0; padding:10px 8px; z-index:100; border-bottom:1px; display:flex; align-items:flex-start; gap:30px;">
    <div style="display:flex; flex-direction:column; gap:6px; align-items:flex-start;">
      <div style="font-weight:700;">Input mode:</div>
      <select id="inputMode" onchange="toggleInputMode()" class="dropdown">
        <option value="">- SELECT -</option>
        <option value="decimal">Decimal</option>
        <option value="dms">DMS</option>
      </select>
    </div>
    
    <div style="display:flex; align-items:flex-start; gap:10px; margin-left:6px;">
      <div style="text-align:left; display:flex; flex-direction:column; gap:4px;">
        <div style="font-weight:700;">Latitude</div>
        <select name="lat" id="lat" class="dropdown">
          <option value="null">- SELECT -</option>
          <option value="n">N - North</option>
          <option value="s">S - South</option>
        </select>
      </div>
      
      <div style="text-align:left; display:flex; flex-direction:column; gap:4px;">
        <div style="font-weight:700;">Longitude</div>
        <select name="lon" id="lon" class="dropdown">
          <option value="null">- SELECT -</option>
          <option value="e">E - East</option>
          <option value="w">W - West</option>
        </select>
      </div>
    </div>
  </div>

<div id="dmsContainer" style="display:none;">
  <h2>DMS INPUT</h2>
  <div id="dmsSection">
<div style="display: inline-grid; grid-template-columns: auto auto; gap: 30px;">
  <div>
  <b>Latitude</b><br>
  <div style="display: inline-flex; align-items: center; gap: 6px;">
    <input type="number" id="latDeg" style="width:45px;" max="90" min="0" oninput="onDmsInput('lat')"> <span>°</span>
    <input type="number" id="latMin" style="width:45px;" max="59" min="0" oninput="onDmsInput('lat')"> <span>'</span>
    <input type="number" id="latSec" style="width:45px;" max="59" min="0" oninput="onDmsInput('lat')"> <span>.</span>
    <input type="number" id="latSecDec" style="width:45px;" min="0" oninput="onDmsInput('lat')"> <span>″</span>
  </div>
</div>

  <div>
  <b>Longitude</b><br>
  <div style="display: inline-flex; align-items: center; gap: 6px;">
    <input type="number" id="lonDeg" style="width:45px;" max="180" min="0" oninput="onDmsInput('lon')"> <span>°</span>
    <input type="number" id="lonMin" style="width:45px;" max="59" min="0" oninput="onDmsInput('lon')"> <span>'</span>
    <input type="number" id="lonSec" style="width:45px;" max="59" min="0" oninput="onDmsInput('lon')"> <span>.</span>
    <input type="number" id="lonSecDec" style="width:45px;" min="0" oninput="onDmsInput('lon')"> <span>″</span>
  </div>
</div>
</div>
</div>
  </div>

<div id="decimalContainer" style="display:none;">
  <h2>DECIMAL INPUT</h2>
  <div id="decimalSection">
  <div style="display: inline-flex; align-items: center; gap: 90px;">
    <div>
      <b>Latitude</b><br>
        <input type="number" style="width: 200px;" value="0" id="latBox" step="0.0001" min="0" max="90" oninput="sliderFunc('latBox', 'latSlider')" onblur="checkBoxRange('latBox','lat')"> <br>
        <input type="range" style="width: 200px;" value="0" id="latSlider" step="0.0001" min="0" max="90" oninput="sliderFunc('latSlider', 'latBox')">
    </div>
      <div>
      <b>Longitude</b><br>
        <input type="number" style="width: 200px;" value="0" id="lonBox" step="0.0001" min="0" max="180" oninput="sliderFunc('lonBox', 'lonSlider')" onblur="checkBoxRange('lonBox','lon')"> <br>
        <input type="range" style="width: 200px;" value="0" id="lonSlider" step="0.0001" min="0" max="180" oninput="sliderFunc('lonSlider', 'lonBox')">
      </div>
  </div>
</div>
</div>

<div> <br>
    	<input type="button" value="UPLOAD COORDINATES" onclick="upload_coords()">
    <p id="status"></p>
  <p id="decimal_output"></p>
  <p id="dms_output"></p>

  <div style="margin-top:12px;">
    <label for="pasteBox"><b>Paste coordinates (temporary):</b></label><br>
    <textarea id="pasteBox" rows="5" style="width:100%; font-size:16px; padding:8px; box-sizing:border-box;" placeholder="Paste coordinates here:"></textarea><br>
    <input type="button" style="margin-top:8px;" value="Apply to inputs (works only for decimal input)" onclick="applyPaste()">
  </div>
<hr style="margin-top:40px; margin-bottom:20px;">

<h2>Battery Status</h2>

<div id="batteryContainer" style="display:flex; gap:40px; flex-wrap:wrap; font-size:18px;">
  
  <div>
    <b>Battery Voltage</b><br>
    <span id="batVoltage">--</span> V<br>
    <span id="batPercent">--</span> %
  </div>

  <div>
    <b>Charging Current</b><br>
    <span id="chargeCurrent">--</span> mA
  </div>

  <div>
    <b>Temperature</b><br>
    <span id="temperature">--</span> °C
  </div>

</div>

<style>
  .dropdown {
    font-family: Verdana;
    font-size: 16px;
    padding: 6px 12px;
    min-width: 120px;
    height: 40px;
    vertical-align: middle;
    display: block;
    box-sizing: border-box;
  }
</style>

<script>
    function sliderFunc(inputIn, inputOut) {
      var val = document.getElementById(inputIn).value
      //document.getElementById(inputOut).value = Math.round((Math.abs(val) * scale)).toString()
        document.getElementById(inputOut).value = Math.abs(val).toFixed(4);
        if (inputIn.indexOf('lat') === 0) convertDecimalToDmsFor('lat');
        if (inputIn.indexOf('lon') === 0) convertDecimalToDmsFor('lon');
        updateDecimalOutput();
    }
      function upload_coords() {
        var mode = document.getElementById('inputMode').value;
        var lat, lon;
        if (mode === 'dms') {
            lat = dmsToDecimal('lat');
            lon = dmsToDecimal('lon');
            // validate absolute values entered in DMS (deg/min/sec -> absolute decimal)
            var vErr = validateCoords(Math.abs(lat), Math.abs(lon));
            if (vErr) { alert(vErr); return; }
          } else {
            // read raw user-entered box values (positive 0.. ranges) and validate those
            var rawLat = parseFloat(document.getElementById('latBox').value) || 0;
            var rawLon = parseFloat(document.getElementById('lonBox').value) || 0;
            var vErr = validateCoords(rawLat, rawLon);
            if (vErr) { alert(vErr); return; }
            lat = rawLat;
            lon = rawLon;
            // apply hemisphere sign after validation
            if (document.getElementById('lat').value === 's') lat = -Math.abs(lat);
            if (document.getElementById('lon').value === 'w') lon = -Math.abs(lon);
          }
          var lat_string = lat.toFixed(4);
          var lon_string = lon.toFixed(4);
          fetch('/coords?lat=' + lat_string + '&lon=' + lon_string)
          .then(() => {
            document.getElementById('status').innerText = 'Sent: lat=' + lat_string + ', lon=' + lon_string;
          })
          .catch(() => {
            document.getElementById('status').innerText = 'Failed to send coordinates';
          });
      }

    function parseFractionalPart(part) {
      if (!part && part !== 0) return 0;
      var s = part.toString();
      if (s.indexOf('.') >= 0) return parseFloat(s);
      if (s === '') return 0;
      return parseFloat('0.' + s);
    }

    function dmsToDecimal(axis) {
      var deg = parseFloat(document.getElementById(axis + 'Deg').value) || 0;
      var min = parseFloat(document.getElementById(axis + 'Min').value) || 0;
      var sec = parseFloat(document.getElementById(axis + 'Sec').value) || 0;
      var secDecRaw = document.getElementById(axis + 'SecDec').value || '';
      var secDec = parseFractionalPart(secDecRaw);
      var seconds = sec + secDec;
      var dec = deg + (min / 60) + (seconds / 3600);
      if (axis === 'lat') {
        if (document.getElementById('lat').value === 's') dec = -dec;
      } else if (axis === 'lon') {
        if (document.getElementById('lon').value === 'w') dec = -dec;
      }
      return dec;
    }

    function convertDmsToDecimalFor(axis) {
      var dec = dmsToDecimal(axis);
      var boxId = (axis === 'lat') ? 'latBox' : 'lonBox';
      document.getElementById(boxId).value = Math.abs(dec).toFixed(4);
      updateDecimalOutput();
    }

    function convertDecimalToDmsFor(axis) {
      var boxId = (axis === 'lat') ? 'latBox' : 'lonBox';
      var val = parseFloat(document.getElementById(boxId).value) || 0;
      var absVal = Math.abs(val);
      var deg = Math.floor(absVal);
      var rem = (absVal - deg) * 60;
      var mins = Math.floor(rem);
      var secs = (rem - mins) * 60;
      var secInt = Math.floor(secs);
      var secFrac = secs - secInt;
      var secDecStr = secFrac.toFixed(3).split('.')[1];
      document.getElementById(axis + 'Deg').value = deg;
      document.getElementById(axis + 'Min').value = mins;
      document.getElementById(axis + 'Sec').value = secInt;
      document.getElementById(axis + 'SecDec').value = secDecStr;
      updateDmsOutput();
    }

    function onDmsInput(axis) {
      convertDmsToDecimalFor(axis);
    }

    function updateDecimalOutput() {
      var latVal = document.getElementById('latBox').value || '0';
      var lonVal = document.getElementById('lonBox').value || '0';
      document.getElementById('decimal_output').innerText = 'Decimal output: ' + latVal + ', ' + lonVal;
    }

    function updateDmsOutput() {
      var latD = document.getElementById('latDeg').value || '0';
      var latM = document.getElementById('latMin').value || '0';
      var latS = document.getElementById('latSec').value || '0';
      var latSD = document.getElementById('latSecDec').value || '0';
      var lonD = document.getElementById('lonDeg').value || '0';
      var lonM = document.getElementById('lonMin').value || '0';
      var lonS = document.getElementById('lonSec').value || '0';
      var lonSD = document.getElementById('lonSecDec').value || '0';
      document.getElementById('dms_output').innerText = 'DMS output: ' + latD + '° ' + latM + "' " + latS + '.' + latSD + '″, ' + lonD + '° ' + lonM + "' " + lonS + '.' + lonSD + '″';
    }

    function toggleInputMode() {
      var mode = document.getElementById('inputMode').value;
      var decC = document.getElementById('decimalContainer');
      var dmsC = document.getElementById('dmsContainer');
      if (decC) decC.style.display = (mode === 'decimal') ? 'block' : 'none';
      if (dmsC) dmsC.style.display = (mode === 'dms') ? 'block' : 'none';
    }

    document.addEventListener('DOMContentLoaded', function() {
      var sel = document.getElementById('inputMode');
      if (sel) {
        if (sel.value === undefined) sel.value = '';
      }
    });

    function applyPaste() {
      var txt = document.getElementById('pasteBox').value || '';
      var m = txt.match(/-?\d+\.?\d*/g);
      if (!m || m.length < 2) {
        alert('Please paste two decimal coordinates (lat, lon)');
        return;
      }
      var lat = parseFloat(m[0]);
      var lon = parseFloat(m[1]);
      document.getElementById('latBox').value = Math.abs(lat).toFixed(4);
      document.getElementById('lonBox').value = Math.abs(lon).toFixed(4);
      document.getElementById('lat').value = (lat < 0) ? 's' : 'n';
      document.getElementById('lon').value = (lon < 0) ? 'w' : 'e';
      updateDecimalOutput();
      convertDecimalToDmsFor('lat');
      convertDecimalToDmsFor('lon');
    }

    function validateCoords(lat, lon) {
      if (!isFinite(lat) || !isFinite(lon)) return 'Coordinates must be numeric values';
      if (lat < 0 || lat > 90) return 'Latitude must be between 0 and 90';
      if (lon < 0 || lon > 180) return 'Longitude must be between 0 and 180';
      return '';
    }

	function checkBoxRange(boxId, axis) {
      var el = document.getElementById(boxId);
      var v = parseFloat(el.value);
      if (!isFinite(v)) {
        alert('Please enter a numeric value');
        el.value = '0.0000';
        return;
      }
      var max = (axis === 'lat') ? 90 : 180;
      if (v < 0 || v > max) {
        var clamped = Math.min(Math.max(v, 0), max);
        el.value = Math.abs(clamped).toFixed(4);
        alert((axis === 'lat' ? 'Latitude' : 'Longitude') + ' must be between 0 and ' + max + '. Value adjusted.');
      }
    }
    
	function updateBatteryStatus(voltage, current, temp) {
  		document.getElementById("batVoltage").innerText = voltage.toFixed(2);
  		var percent = ((voltage - 3.0) / (4.2 - 3.0)) * 100;
  		percent = Math.max(0, Math.min(100, percent));
  		document.getElementById("batPercent").innerText = percent.toFixed(0);
  		document.getElementById("batBar").style.width = percent + "%";
  		document.getElementById("chargeCurrent").innerText = current.toFixed(0);
  		document.getElementById("temperature").innerText = temp.toFixed(1);
}
	function fetchBattery() {
  		fetch('/battery')
    	.then(response => response.json())
    	.then(data => {
     	updateBatteryStatus(data.voltage, data.current, data.temperature);
    	})
    	.catch(() => {});
}

setInterval(fetchBattery, 5000);
</script>
)rawliteral";
/* ============================================================ */
