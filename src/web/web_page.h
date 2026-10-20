#pragma once

// REAL sensor monitoring + configuration dashboard; no synthetic sensor controls.
// HTML/CSS/JS are served by ESP32-S3, no cloud or external frameworks.
const char WEB_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32-S3 Smart Home | REAL Monitoring</title>
<style>
:root {color-scheme: dark;--bg:#101827;--card:#1e2b3e;--sub:#172235;--line:#40516a;--text:#e9f0fb;--muted:#acbdd2;--blue:#5eafff;--ok:#84d7aa;--yellow:#f1ca79}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font:15px/1.55 system-ui,-apple-system,"Segoe UI",sans-serif}
main{max-width:1120px;margin:0 auto;padding:24px 18px 50px}
header{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-bottom:20px}
h1{margin:0;font-size:28px;letter-spacing:-.7px} h2{font-size:18px;margin:0 0 12px} h3{font-size:15px;margin:0 0 10px}
p{margin:6px 0}.muted,.note{color:var(--muted)}.note{font-size:12px;line-height:1.55;margin-top:12px}
.pill{background:#1c3a32;border:1px solid #45745e;color:#b0efd0;font-size:12px;padding:5px 11px;border-radius:99px;white-space:nowrap}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:14px;align-items:start}
.card{background:var(--card);border:1px solid #37465b;border-radius:14px;padding:18px;min-width:0}
.wide{grid-column:1/-1}
.statgrid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:9px}
.stat{padding:11px 12px;border-radius:9px;background:var(--sub);border:1px solid #2c3c50;min-width:0}
.key{color:var(--muted);font-size:12px;margin-bottom:4px}
.val{font-size:19px;font-weight:650;font-variant-numeric:tabular-nums;overflow-wrap:anywhere}
.rule{margin:18px 0;border:0;border-top:1px solid var(--line)}
.metricgrid{grid-template-columns:repeat(4,minmax(0,1fr))}
.setting{margin:16px 0;padding:13px;background:var(--sub);border:1px solid #334862;border-radius:10px}
.settingtop{display:flex;align-items:center;justify-content:space-between;gap:12px}
.setting .amount{color:#b9ddff;font-size:20px;font-weight:650;font-variant-numeric:tabular-nums;white-space:nowrap}
input[type=range]{display:block;width:100%;margin:13px 0 0;accent-color:var(--blue)}
.subnote{font-size:12px;color:var(--muted);margin:3px 0 0}
button{padding:10px 16px;border-radius:8px;border:0;background:#5eb2ff;color:#082138;font-size:14px;font-weight:650;cursor:pointer}
button:disabled{opacity:.6;cursor:wait}
.status{font-size:13px;color:var(--ok);min-height:20px;margin-top:10px}
table{width:100%;border-collapse:collapse}th,td{padding:9px 10px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}td:last-child{text-align:right;font-variant-numeric:tabular-nums} th{font-size:13px;font-weight:600;color:var(--muted)}
.fixed{border:1px solid #6c624a;background:#342f26;padding:10px;border-radius:8px;color:#f1dfb0;font-size:13px}
@media(max-width:760px){.grid{grid-template-columns:1fr}.wide{grid-column:auto}.metricgrid{grid-template-columns:repeat(2,minmax(0,1fr))}header{flex-wrap:wrap}main{padding:14px}h1{font-size:23px}}
</style>
</head>
<body>
<main>
<header>
  <div><h1>ESP32-S3 Smart Home</h1><p class="muted">FreeRTOS · Theo dõi cảm biến REAL · Điều khiển tự động theo luật</p></div>
  <span class="pill" id="connection">Đang kết nối...</span>
</header>
<div class="grid">
  <section class="card">
    <h2>Giám sát môi trường</h2>
    <p class="note" style="margin:-3px 0 14px">Giá trị từ DHT22, LDR và PIR trong Wokwi; không nhập giả lập từ Web.</p>
    <div class="statgrid">
      <div class="stat"><div class="key">Nhiệt độ DHT22</div><div class="val" id="temp">—</div></div>
      <div class="stat"><div class="key">Độ ẩm DHT22</div><div class="val" id="hum">—</div></div>
      <div class="stat"><div class="key">Độ rọi LDR (ước tính)</div><div class="val" id="lux">—</div></div>
      <div class="stat"><div class="key">Mức sáng quy ước</div><div class="val" id="light">—</div></div>
      <div class="stat"><div class="key">Cảm biến PIR</div><div class="val" id="occ">—</div></div>
      <div class="stat"><div class="key">Nguồn dữ liệu</div><div class="val">REAL</div></div>
    </div>
    <hr class="rule">
    <h3>Đối chiếu dữ liệu LDR</h3>
    <table aria-label="Đại lượng LDR"><tbody>
      <tr><td>ADC1_CH3 · GPIO4</td><td id="adc">—</td></tr>
      <tr><td>Điện áp AO (mô hình)</td><td id="volt">—</td></tr>
      <tr><td>Điện trở quang trở (mô hình)</td><td id="res">—</td></tr>
    </tbody></table>
    <p class="note">Mã ADC lấy từ <code>analogRead(GPIO4)</code>. Điện áp, điện trở và lux được <b>ước tính</b> theo mô hình LDR mặc định của Wokwi; không phải phép đo điện áp đã hiệu chuẩn.</p>
  </section>
  <section class="card">
    <h2>Cấu hình điều khiển tự động</h2>
    <p class="muted">Chỉ điều chỉnh ngưỡng quyết định, không thay đổi đầu vào cảm biến.</p>
    <div class="setting">
      <div class="settingtop"><label for="cooling">Ngưỡng kích hoạt COOLING</label><span class="amount"><span id="cv">30</span> °C</span></div>
      <input id="cooling" type="range" min="25" max="40" step="1" value="30">
      <p class="subnote">Có người và nhiệt độ từ ngưỡng này trở lên → làm mát.</p>
    </div>
    <div class="setting">
      <div class="settingtop"><label for="lighting">Ngưỡng kích hoạt LIGHTING</label><span class="amount"><span id="lv">30</span>%</span></div>
      <input id="lighting" type="range" min="10" max="60" step="1" value="30">
      <p class="subnote">Có người, không cần làm mát và sáng dưới ngưỡng → bật đèn.</p>
    </div>
    <div class="fixed">Ngưỡng an toàn WARNING: <strong>45 °C, cố định trong firmware.</strong> Cảnh báo được ưu tiên cao nhất và không thể chỉnh sửa từ Web.</div>
    <p class="note">Độ ưu tiên: WARNING → ENERGY_SAVING (PIR = 0) → COOLING → LIGHTING → COMFORT.</p>
    <button id="save" type="button">Lưu cấu hình</button>
    <div id="configStatus" class="status" role="status"></div>
    <p class="subnote">Cấu hình chỉ tồn tại trong RAM; khi khởi động lại ESP32-S3, hai ngưỡng về mặc định 30 °C và 30%.</p>
    <hr class="rule">
    <h3>Trạng thái điều khiển</h3>
    <div class="statgrid">
      <div class="stat"><div class="key">Chế độ điều khiển</div><div class="val" id="mode">—</div></div>
      <div class="stat"><div class="key">Relay đèn</div><div class="val" id="outLight">—</div></div>
      <div class="stat"><div class="key">LED mô phỏng quạt</div><div class="val" id="fan">—</div></div>
      <div class="stat"><div class="key">Buzzer</div><div class="val" id="alarm">—</div></div>
    </div>
  </section>
  <section class="card wide">
    <h2>Chỉ số thời gian thực trong mô phỏng</h2>
    <div class="statgrid metricgrid">
      <div class="stat"><div class="key">PIR ISR → Task</div><div class="val" id="pir">—</div></div>
      <div class="stat"><div class="key">Sensor → Actuator</div><div class="val" id="resp">—</div></div>
      <div class="stat"><div class="key">Thực thi ControlTask</div><div class="val" id="exec">—</div></div>
      <div class="stat"><div class="key">Tỷ lệ đạt deadline</div><div class="val" id="deadline">—</div></div>
    </div>
    <p class="note">Các số đo chỉ có ý nghĩa trong phiên chạy mô phỏng. Deadline được tính theo thời gian từ mẫu dữ liệu mới đến khi ControlTask cập nhật đầu ra; không phải cam kết thời gian đáp ứng của phần cứng thực tế.</p>
  </section>
</div>
</main>
<script>
const $ = id => document.getElementById(id);
const cooling=$('cooling'),lighting=$('lighting'),save=$('save');
let lastSavedRevision=0;
const set=(id,value)=>$(id).textContent=value;
function showInput(){set('cv',cooling.value);set('lv',lighting.value)}
cooling.addEventListener('input',showInput);lighting.addEventListener('input',showInput);
showInput();
function status(text,bad=false){set('configStatus',text);$('configStatus').style.color=bad?'#ffb7b7':'#96e3b9'}
async function api(url,opt){const response=await fetch(url,opt);let data={};try{data=await response.json()}catch(e){}if(!response.ok)throw new Error(data.error||('HTTP '+response.status));return data}
async function loadConfig(){
  const c=await api('/api/config',{cache:'no-store'});
  cooling.value=c.coolingTempC;lighting.value=c.darkLightPct;lastSavedRevision=c.revision;
  showInput();
}
save.addEventListener('click',async()=>{
  save.disabled=true;
  try{
    const body=new URLSearchParams({coolingTempC:cooling.value,darkLightPct:lighting.value});
    const c=await api('/api/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
    lastSavedRevision=c.revision;
    status('Đã lưu cấu hình v'+c.revision+'. Chờ DecisionTask áp dụng...');
  }catch(e){status('Lưu thất bại: '+e.message,true)}
  finally{save.disabled=false}
});
function n(v,d=1){return Number(v).toFixed(d)}
async function refresh(){
 try{
   const s=await api('/api/state',{cache:'no-store'});
   set('connection','Đã kết nối · Cảm biến REAL');
   $('connection').style.background='#1c3a32';
   if(!s.sensorReady){set('temp','Đang đọc...');return}
   set('temp',n(s.temperature)+' °C');set('hum',n(s.humidity)+' %RH');
   set('lux',n(s.lightLux)+' lux');set('light',n(s.light)+'%');
   set('occ',s.occupancy?'Có tín hiệu (1)':'Không (0)');
   set('adc',s.ldrAdc+' / 4095');set('volt',n(s.ldrVoltage,3)+' V');set('res',n(s.ldrResistance,2)+' kΩ');
   set('mode',s.decisionReady?s.mode:'Đang quyết định');
   set('outLight',s.lightOn?'BẬT':'TẮT');set('fan',s.fan+' %');set('alarm',s.alarmOn?'BẬT':'TẮT');
   set('pir',s.pirLatency+' ms');set('resp',s.responseTime+' ms');
   set('exec',n(s.controlExec,3)+' ms');set('deadline',s.controlSamples?s.deadlineRate+' %':'Chưa có mẫu');
   if(lastSavedRevision && s.appliedRevision>=lastSavedRevision && $('configStatus').textContent.includes('Chờ')){
     status('Cấu hình v'+lastSavedRevision+' đã được DecisionTask áp dụng.');
   }
 }catch(e){set('connection','Mất kết nối');$('connection').style.background='#633432'}
}
loadConfig().catch(e=>status('Không đọc được cấu hình: '+e.message,true));
refresh();setInterval(refresh,500);
</script>
</body>
</html>)HTML";
