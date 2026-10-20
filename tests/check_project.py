"""Static Wokwi pin mapping, HTTP routes, queue and configuration checks.
It does not compile ESP32 firmware or prove scheduling/peripheral behavior on Wokwi.
"""
from pathlib import Path
import json, re
root=Path(__file__).resolve().parents[1]
cfg=(root/'src/core/config.h').read_text()
diagram=json.loads((root/'diagram.json').read_text())
assert len({x['id'] for x in diagram['parts']})==len(diagram['parts']),'Duplicate parts'
conns={tuple(sorted((x[0],x[1]))) for x in diagram['connections']}
checks=[('DHT_PIN','dht1:SDA','esp1:15'),('LDR_PIN','ldr1:AO','esp1:4'),
        ('PIR_PIN','pir1:OUT','esp1:16'),('LIGHT_PIN','relayLight:IN','esp1:6'),
        ('FAN_PIN','esp1:17','rFan:1'),('BUZZER_PIN','esp1:7','buzzer1:2'),
        ('OLED_SDA','oled1:SDA','esp1:8'),('OLED_SCL','oled1:SCL','esp1:9')]
for key,x,y in checks:
    assert re.search(r'#define\s+'+key+r'\s+\d+',cfg),key
    assert tuple(sorted((x,y))) in conns,(x,y)
web=(root/'src/web/web_server.cpp').read_text()
for uri,method in [('/','HTTP_GET'),('/api/state','HTTP_GET'),
                   ('/api/config','HTTP_GET'),('/api/config','HTTP_POST')]:
    assert f'server.on("{uri}", {method}' in web, (uri,method)
for route in ['/api/sensors','/api/real']:
    assert route not in web, ('old simulated route present', route)
state=(root/'src/core/app_state.cpp').read_text()
assert 'xQueueCreate(1, sizeof(SensorData))' in state
assert 'xQueueCreate(1, sizeof(DecisionData))' in state
assert 'xSemaphoreCreateBinary()' in state
assert 'xSemaphoreCreateMutex()' in state
assert 'controllerSettings.revision++' in state
assert 'TEMP_WARNING = 45.0f' in cfg
assert 'warningTempC' not in state
sensor=(root/'src/sensors/sensors.cpp').read_text()
assert 'ldrFromAdc(raw)' in sensor and 'WEB_SENSOR' not in sensor
control=(root/'src/control/control.cpp').read_text()
assert 'decision.sensor' in control and 'decision.settings.darkLightPct' in control
assert 'responseTimeMs > CONTROL_DEADLINE_MS' in control
page=(root/'src/web/web_page.h').read_text()
assert 'POST' in page and 'coolingTempC' in page and 'darkLightPct' in page
assert 'Use Web Values' not in page and "'/api/sensors'" not in page
assert 'espressif32 @ 6.12.0' in (root/'platformio.ini').read_text()
print('PASS: pin mapping, HTTP methods, mutex config, FreeRTOS objects, no WEB source')
