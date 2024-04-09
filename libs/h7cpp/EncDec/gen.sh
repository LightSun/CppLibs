#1、生成私钥，保存在文件rsa_private_key.pem里面
openssl genrsa -out rsa_private_key.pem 1024
 
#2、通过私钥生成公钥，保存在文件rsa_private_key.pem里面
openssl rsa -in rsa_private_key.pem -pubout -out rsa_public_key.pem
