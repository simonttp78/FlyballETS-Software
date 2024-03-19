# extra_scripts = ota_web_upload.py
# upload_protocol = custom
# custom_upload_url = http://192.168.20.1/doupdate


import requests
import hashlib
Import('env')

try:
    from requests_toolbelt import MultipartEncoder, MultipartEncoderMonitor
    from tqdm import tqdm
except ImportError:
    env.Execute("$PYTHONEXE -m pip install requests_toolbelt")
    env.Execute("$PYTHONEXE -m pip install tqdm")
    from requests_toolbelt import MultipartEncoder, MultipartEncoderMonitor
    from tqdm import tqdm

def on_upload(source, target, env):
    firmware_path = str(source[0])
    upload_url = env.GetProjectOption('custom_upload_url')

    with open(firmware_path, 'rb') as firmware:
        md5 = hashlib.md5(firmware.read()).hexdigest()
        firmware.seek(0)
        encoder = MultipartEncoder(fields={
            'MD5': md5, 
            'firmware': ('firmware', firmware, 'application/octet-stream')}
        )

        bar = tqdm(desc='Upload Progress',
              total=encoder.len,
              dynamic_ncols=True,
              unit='B',
              unit_scale=True,
              unit_divisor=1024
              )

        monitor = MultipartEncoderMonitor(encoder, lambda monitor: bar.update(monitor.bytes_read - bar.n))

        response = requests.post(upload_url, data=monitor, headers={'Content-Type': monitor.content_type})
        bar.close()
        print(response,response.text)
            
env.Replace(UPLOADCMD=on_upload)