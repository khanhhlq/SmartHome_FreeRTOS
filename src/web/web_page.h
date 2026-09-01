#pragma once

// HTML được tách khỏi main.cpp để code embedded dễ đọc hơn.
const char WEB_PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP32 Smart Home</title>
  <style>
    body{font-family:Arial;background:#0f172a;color:#e2e8f0;margin:0}
    main{max-width:850px;margin:auto;padding:20px}
    section{background:#1e293b;padding:18px;border-radius:14px;margin-bottom:16px}
    .grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}
    label{display:block;margin:12px 0}
    input[type=range]{width:100%}
    button{padding:10px 14px;margin:5px;border:0;border-radius:8px;cursor:pointer}
    .value{font-weight:bold}
    @media(max-width:650px){.grid{grid-template-columns:1fr}}
  </style>
</head>
<body>
<main>
  <h1>ESP32-S3 Smart Home</h1>
  <p>FreeRTOS + FSM / Rule-Based Control</p>

  <div class="grid">
    <section>
      <h2>Web Sensor Input</h2>

      <label>Temperature: <span id="tv">25</span> °C
        <input id="t" type="range" min="20" max="60" step="1" value="25">
      </label>

      <label>Humidity: <span id="hv">55</span> %
        <input id="h" type="range" min="30" max="95" value="55">
      </label>

      <label>Light: <span id="lv">65</span> %
        <input id="l" type="range" min="0" max="100" value="65">
      </label>

      <label>
        <input id="o" type="checkbox" checked>
        Occupancy
      </label>

      <button id="send">Use Web Values</button>
      <button id="real">Use Real Sensors</button>
    </section>

    <section>
      <h2>System State</h2>
      <p>Source: <b id="source">-</b></p>
      <p>Mode: <b id="mode">-</b></p>
      <p>Temperature: <span id="temp">-</span></p>
      <p>Humidity: <span id="hum">-</span></p>
      <p>Light: <span id="light">-</span></p>
      <p>Occupancy: <span id="occ">-</span></p>
      <hr>
      <p>Light output: <b id="outLight">-</b></p>
      <p>Fan: <b id="fan">-</b></p>
      <p>Alarm: <b id="alarm">-</b></p>
      <hr>
      <h3>Real-time metrics</h3>
      <p>PIR ISR → Task: <b id="pir">-</b> ms</p>
      <p>Sensor → Actuator: <b id="response">-</b> ms</p>
      <p>Control execution: <b id="exec">-</b> ms</p>
      <p>Control deadline success: <b id="deadline">-</b> %</p>
    </section>
  </div>

<script>
const $ = id => document.getElementById(id);
const t=$('t'),h=$('h'),l=$('l'),o=$('o');

function updateLabels(){
  $('tv').textContent=t.value;
  $('hv').textContent=h.value;
  $('lv').textContent=l.value;
}
[t,h,l].forEach(x=>x.addEventListener('input',updateLabels));
updateLabels();

async function post(url,data={}){
  const body=new URLSearchParams(data);
  await fetch(url,{
    method:'POST',
    headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body
  });
}

$('send').onclick=async()=>{
  await post('/api/sensors',{
    temperature:t.value,
    humidity:h.value,
    light:l.value,
    occupancy:o.checked?1:0
  });
};

$('real').onclick=()=>post('/api/real');

async function refresh(){
  try{
    const s=await (await fetch('/api/state',{cache:'no-store'})).json();

    $('source').textContent=s.source;
    $('mode').textContent=s.mode;
    $('temp').textContent=s.temperature+' °C';
    $('hum').textContent=s.humidity+' %';
    $('light').textContent=s.light+' %';
    $('occ').textContent=s.occupancy?'YES':'NO';

    $('outLight').textContent=s.lightOn?'ON':'OFF';
    $('fan').textContent=s.fan+' %';
    $('alarm').textContent=s.alarmOn?'ON':'OFF';

    $('pir').textContent=s.pirLatency;
    $('response').textContent=s.responseTime;
    $('exec').textContent=s.controlExec;
    $('deadline').textContent=s.deadlineRate;
  }catch(e){}
}

refresh();
setInterval(refresh,500);
</script>
</main>
</body>
</html>
)HTML";
