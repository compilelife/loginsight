# -*- coding:utf-8 -*-
import os
import time
from webdav3.client import Client
import zipfile

def zip_dir(dirname,zipfilename):
  filelist = []
  if os.path.isfile(dirname):
    filelist.append(dirname)
  else :
    for root, dirs, files in os.walk(dirname):
      for name in files:
        filelist.append(os.path.join(root, name))
  zf = zipfile.ZipFile(zipfilename, "w", zipfile.zlib.DEFLATED)
  for tar in filelist:
    arcname = tar[len(dirname):]
    #print arcname
    zf.write(tar,arcname)
  zf.close()

driver_prefix = 'nutstore'

webdav_url = os.getenv(f"{driver_prefix}_url")
webdav_username = os.getenv(f"{driver_prefix}_username")
webdav_password = os.getenv(f"{driver_prefix}_password")

if webdav_username==None or webdav_username=="" or webdav_password==None or webdav_password=="":
    print(f"{driver_prefix}_username 或 {driver_prefix}_password 没有配置secret")
    exit(-1)

options = {
 'webdav_hostname': webdav_url,
 'webdav_login':    webdav_username,
 'webdav_password': webdav_password,
 'disable_check': True
}

client = Client(options)

filename=os.getenv("filename")

zip_dir('build/artifact', filename)

remotedir = '我的坚果云/'+time.strftime("%Y-%m-%d %H%M%S", time.localtime())+'/'
client.mkdir(remotedir)

client.upload(remote_path=remotedir+filename,local_path=filename)
print('done upload')