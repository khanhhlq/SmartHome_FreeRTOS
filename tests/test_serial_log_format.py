from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
files = [root/'src/sensors/sensors.cpp', root/'src/pir/pir.cpp',root/'src/web/web_server.cpp']
for file in files:
    txt=file.read_text(encoding='utf-8')
    printf_groups=re.findall(r'Serial\.printf\s*\((.*?)\);',txt,re.S)
    assert printf_groups, f'No Serial.printf: {file}'
    for group in printf_groups:
        # A raw LF escape in Serial.printf can make the terminal cursor step diagonally.
        assert not re.search(r'(?<!\\)\\n',group.replace(r'\r\n','')), file
        assert r'\r\n' in group, file
assert 'ldrResistanceKOhm' in (root/'src/sensors/sensors.cpp').read_text()
assert 'Light(FSM)' in (root/'src/sensors/sensors.cpp').read_text()
print('PASS: all 3 dynamic Serial.printf use CRLF; LDR diagnostic pipeline present')
