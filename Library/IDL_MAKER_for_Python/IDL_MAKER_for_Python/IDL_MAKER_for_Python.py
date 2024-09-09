# -*- coding: utf-8 -*-

###### pip install parse ####
from re import A
from parse import *

KeyWordTable = ["#PACKETNUM",  "#NOBUFF", "#DEST", "#struct", "#define"]

PACKETNUM : int = 0

DefineDisableWarning : str = "#pragma warning( disable : 4309 ) \n#pragma warning( disable : 4267 )"
DefineHeaderSTRUCT :str = "st_PACKET_HEADER"
DefineForwardDeclaration : str = "void ForwardDecl(int DestID, SerializeBuffer *sb);"
DefineSessionValue : str = "SrcID"
DefineSessionStr : str = "int " + DefineSessionValue
DefineSerializeBufferValue : str = "p_Message"
DefineSerializeBufferStr : str = "SerializeBuffer* " + DefineSerializeBufferValue
DefineTypeValue : str = "Packet_Type"
DefineTypeStr : str = "char " + DefineTypeValue


ProxyDestStr : str          = ""

ProxyCppDefine : str        = "OreoPizza::Proxy::"
StubCppDefine : str         = "OreoPizza::Stub::"
StubCoverDefine : str       = "Message_" 
StubConverParam : str       = "(" + DefineSessionStr +", " + DefineSerializeBufferStr + ")"

HeaderType : str = ""
PacketDefineHeaderStr : str = ""
ProxyHeaderStr : str = ""
ProxyCppStr : str = ""
StubMessageParam : str = DefineSessionStr + ", " + DefineTypeStr + ", " +  DefineSerializeBufferStr
StubHeaderStr: str = "\tprivate: \tvoid ProcessRecvMessage(" + StubMessageParam + "); \n\n"
StubCppStr : str = ""
StubCppSwitchStr : str = ""


def WritePacketDefineHeader():
    global PacketDefineHeaderStr
    ########################################################
    # ./PacketDefine.h 파일 작성 
    PacketDefineHeader = open("./Protocol.h", "w", encoding="utf-16") 
    PacketDefineHeader.write("#ifndef __PROTOCOL_H__ \n")
    PacketDefineHeader.write("#define __PROTOCOL_H__ \n\n") 
    PacketDefineHeader.write(PacketDefineHeaderStr)
    PacketDefineHeader.write("\n#endif")
    PacketDefineHeader.close() 
    
def WriteProxyHeader():
    global ProxyHeaderStr
    
    ########################################################
    # ./Proxy.h 파일 작성 
    ProxyHeader = open("./Proxy.h", "w", encoding="utf-16")
    ProxyHeader.write("#ifndef __PROXY_H__ \n")
    ProxyHeader.write("#define __PROXY_H__ \n\n")

    ProxyHeader.write("namespace OreoPizza \n")
    ProxyHeader.write("{ \n")
    
    ProxyHeader.write("\tclass Proxy \n")
    ProxyHeader.write("\t{ \n")
    
    ProxyHeader.write(ProxyHeaderStr)
    
    ProxyHeader.write("\t}; \n")

    ProxyHeader.write("} \n")

    ProxyHeader.write("\nextern OreoPizza::Proxy proxy; \n")
    
    ProxyHeader.write("\n#endif")
    ProxyHeader.close()

def WriteProxyCpp():
    global ProxyCppStr


    ########################################################
    # ./Proxy.cpp 파일 작성
    ProxyCpp = open("./Proxy.cpp", "w", encoding="utf-16")

    ########################################################
    # 직렬화 버퍼 연결
    # Protocol.h 연결
    ProxyCpp.write("#include \"Protocol.h\" \n")
    ProxyCpp.write("#include \"SerializeBuffer.h\" \n")
    ProxyCpp.write("#include \"proxy.h\" \n")
    ProxyCpp.write("\n" + DefineDisableWarning + "\n\n")
    ProxyCpp.write(DefineForwardDeclaration + "\n")
    ProxyCpp.write("OreoPizza::Proxy proxy; \n")


    
    ########################################################
    # 네트워크 라이브러리랑 같이 맞물려 있어야 함.
    # But 현재 네트워크 라이브러리가 없어서 일단 건너 뛴다. 

    ProxyCpp.write("\n")
    ProxyCpp.write(ProxyCppStr)
    ProxyCpp.close()

def WriteStubHeader():
    global StubHeaderStr

    ########################################################
    # ./stup.h 파일 작성
    StubHeader = open("./Stub.h", "w", encoding="utf-16")
    StubHeader.write("#ifndef __STUB_H__ \n")
    StubHeader.write("#define __STUB_H__ \n\n")
    
    StubHeader.write("namespace OreoPizza \n")
    StubHeader.write("{ \n")
    
    StubHeader.write("\tclass Stub \n")
    StubHeader.write("\t{ \n")
    
    # 내가 작성한 내용이 들어간다.
    StubHeader.write(StubHeaderStr)

    StubHeader.write("\t}; \n")

    StubHeader.write("} \n")

    StubHeader.write("\nextern OreoPizza::Stub stub; \n")

    StubHeader.write("#endif \n")
    StubHeader.close()
 
def WriteStubCpp():
    global StubCppStr
    

    StubCpp = open("./Stub.cpp", "w", encoding="utf-16")

    ########################################################
    # 직렬화 버퍼 연결
    StubCpp.write("#include \"SerializeBuffer.h\" \n")
    StubCpp.write("#include \"Protocol.h\" \n")
    StubCpp.write("#include \"Stub.h\" \n")
    StubCpp.write("OreoPizza::Stub stub; \n")
    StubCpp.write("\n")
    # 스위치문 작성
    WriteStubProcessRecvMessage(StubCpp)

    # 실제 함수 작성
    StubCpp.write(StubCppStr)

    StubCpp.close()    

def WriteStubProcessRecvMessage(StubCpp):
    global StubCppSwitchStr
    global DefineTypeValue
    global StubMessageParam

    StubCpp.write("void OreoPizza::Stub::ProcessRecvMessage(" + StubMessageParam + ") \n{ \n")
    StubCpp.write("\tswitch("+ DefineTypeValue +") \n\t{ \n")
    
    # 메시지에서 PacketType 뽑아내기
    #
    
    StubCpp.write(StubCppSwitchStr)

    StubCpp.write("\tdefault: \n")
    # 이상한 타입의 예외처리~~~~
    StubCpp.write("\t\tbreak; \n")
    StubCpp.write("\t} \n")
    StubCpp.write("} \n\n")

def SaveFile():


    ########################################################
    # ./PacketDefine.h 파일 작성 
    WritePacketDefineHeader()
    
    ########################################################
    # ./Proxy.h 파일 작성 
    WriteProxyHeader()

    ########################################################
    # ./Proxy.cpp 파일 작성
    WriteProxyCpp()

    ########################################################
    # ./stup.h 파일 작성
    WriteStubHeader()

    ########################################################
    # ./stup.cpp 파일 작성
    WriteStubCpp()
    return



def removeKeyWord(Param):

    retStr = ""
    TempParam = Param.split(",")
    for subTempParam in TempParam:
        for subString in KeyWordTable:
            subTempParam = subTempParam.replace(subString, "").strip()
        retStr += subTempParam + ", "
    
    retStr = retStr.rstrip(', ')

    return retStr

def PushBuff(Param):
    global KeyWordTable
    global HeaderType
    global DefineHeaderSTRUCT

    TempStr = "SerializeBuffer sb; \n"
    TempStr += "\t" + DefineHeaderSTRUCT + " header; \n\n"

    ####################################################
    # 1. 직렬화 버퍼에 패킷의 헤더를 넣는다. 
    # 2. 직렬화 버퍼에 데이터를 넣는다. 
    # 3. 직렬화 버퍼에 사이즈를 헤더 사이즈 위치에 넣는다.
    TempStr += "\tsb.PutData((char *)&header, sizeof(" + DefineHeaderSTRUCT + ")); \n"
    
    TempParam = Param.split(",")
    TempStr += "\tsb"
    for subTempParam in TempParam:
        subTempParam = subTempParam.strip()
        if MyKeyWord(subTempParam) != -1:
            words = subTempParam.split()
            TempStr += " << " + words[-1]
    TempStr += "; \n\n"

    TempStr += "\theader.byCode = dfPACKET_CODE; \n"
    TempStr += "\theader.byType = " + HeaderType + "; \n"
    TempStr += "\theader.bySize = sb.GetDataSize() - sizeof(" + DefineHeaderSTRUCT + "); \n\n"
    
    
    TempStr += "\tsb.ReWrite(); \n"
    TempStr += "\tsb.PutData((char*)&header, sizeof(st_PACKET_HEADER)); \n"
    TempStr += "\tsb.ReturnPos(); \n\n "


    ##################################################################################
    # 전방선언된 메시지 전송 함수
    # 네트워크 코드에 실제 함수 작성되어 있음
    TempStr += "\tForwardDecl(DestID, &sb); \n"
    
    return TempStr
    
def MyKeyWord(line : str, option = None):
    global PACKETNUM
    global KeyWordTabl
    global PacketDefineHeaderStr

    line = line.strip();

    for subString in KeyWordTable:
        if line.find(subString) != -1:
            if subString == "#PACKETNUM":
                KeyWord, Value = line.split(" ")
                PACKETNUM = int(Value)
                return 
            if subString == "#struct":
                KeyWord, Value = line.split(" ")
                PacketDefineHeaderStr += "struct " + Value + '\n'
                SetProxyDefineSTRUCT(option)
                return
            if subString == "#define" :
                PacketDefineHeaderStr += line + '\n'
                return 
            if subString == "#NOBUFF":
                return -1
    
    return



def SetProxyDefine(FuncName : str):
    global PACKETNUM
    global PacketDefineHeaderStr
    global HeaderType
    
    HeaderType = "df" + FuncName

    PacketDefineHeaderStr += "#define " + HeaderType + "\t\t\t\t\t\t" + str(PACKETNUM) + " \n"
    return 

def SetProxyDefineSTRUCT(StructData):
    global PacketDefineHeaderStr
    
    line = StructData.readline()
    while line.find("};") == -1:
        PacketDefineHeaderStr += line 
        line = StructData.readline()
    PacketDefineHeaderStr += line 
        
    return

def SetProxyHeader(DataType, FuncName, Param):
    global ProxyHeaderStr
    global KeyWordTable
    
    ProxyHeaderStr += "\t\tpublic: \t\t" + DataType + " " + FuncName 

    # 파라미터에서 키워드를 제거하여 만들어야 한다.
    ProxyHeaderStr += "(" + "int destID, " + removeKeyWord(Param) + "); \n"

    return

def SetProxyCpp(DataType, FuncName, Param) :
    global ProxyCppStr
    global ProxyCppDefine
    global KeyWordTable
    

    ProxyCppStr += DataType + " " + ProxyCppDefine + FuncName 

    # 파라미터에서 키워드를 제거한다. 
    ProxyCppStr += "(" + "int DestID, " + removeKeyWord(Param) + ") \n"
        
    ProxyCppStr += "{ \n"

    ########################################################
    # 직렬화 버퍼에 넣는다.
    ProxyCppStr += "\t" + PushBuff(Param) + "\n"
    
    ########################################################
    # Send를 한다. 
    

    ProxyCppStr += "} \n\n"
    return 
    
def SetStubHeader(DataType, FuncName, Param) :
    global StubHeaderStr 
    global StubCoverDefine
    global KeyWordTable
    global StubConverParam
    
    # private 로 이루어진 함수가 메시지를 받아서 메시지를 뜯어내고 
    # protected 로 이루어진 가상함수가 파라미터로 값을 받아서 알아서 처리한다.
    StubHeaderStr += "\tprivate: \tvoid " + StubCoverDefine + FuncName + StubConverParam + "; \n"
    StubHeaderStr += "\tprotected: \tvirtual " + DataType + " " + FuncName 
    
    StubHeaderStr += "(" + DefineSessionStr + ", " + removeKeyWord(Param) + "); \n\n"

    return
    
def SetStubCpp(DataType, FuncName, Param) :
    global StubCppStr
    global StubCppDefine
    global KeyWordTable
    global StubCoverDefine
    global StubConverParam
    
    ########################################################
    # private 껍데기 함수
    StubCppStr += "void " + StubCppDefine + StubCoverDefine + FuncName + StubConverParam + " \n"
    StubCppStr += "{ \n"

    # 메시지를 받을 파라미터 생성 문자열 작성
    TempRemoveKeyWordParamStr = removeKeyWord(Param)
    TempStr : str = "*" + DefineSerializeBufferValue
    TempParamStr : str = DefineSessionValue

    TempParamList = TempRemoveKeyWordParamStr.split(",")
    for TempParam in TempParamList:
        TempParam = TempParam.strip()
        StubCppStr += "\t" + TempParam + "; \n"
        TempDataType, TempValue = TempParam.split()
        TempStr += " >> " + TempValue 
        TempParamStr += ", " + TempValue

    # 실제 문자열에 문자열 입력
    StubCppStr += "\n\t" + TempStr + "; \n\n"
    
    # 메시지를 뜯어서 가상함수에 파라미터로 넘긴다. 
    StubCppStr += "\t" + FuncName + "(" + TempParamStr +"); \n"

    StubCppStr += "} \n\n"
    


    ########################################################
    # 사용자가 실제 사용할 함수 
    StubCppStr += DataType + " " + StubCppDefine + FuncName 
    
    StubCppStr += "(" + DefineSessionStr + ", " + removeKeyWord(Param) + ") \n"

    StubCppStr += "{ \n"
    StubCppStr += "\t return "
    
    if DataType == "void":
        StubCppStr += "; \n"
    else :
        StubCppStr += "false; \n"
        
    StubCppStr += "} \n\n"
        

    

    return 
    
def SetStubCppSwitch(DataType, FuncName, Param):
    global StubCppSwitchStr
    global KeyWordTable
    global StubCoverDefine
    global StubConverParam
    

    StubCppSwitchStr += "\tcase " + "df" + FuncName.upper() + ": \n"
    StubCppSwitchStr += "\t\t" + StubCoverDefine + FuncName + "(" + DefineSessionValue + ", "+ DefineSerializeBufferValue + "); \n"
    
    
    StubCppSwitchStr += "\t\tbreak; \n"

    

    return

    
    

def ParsingFunc(line : str):
    global PACKETNUM

    result = parse("{DataType} {FuncName}({Param})", line)

    DataType = result["DataType"]
    FuncName = result["FuncName"].strip()
    Param   = result["Param"]
    

    # 1. PacketDefine.h 파일에 작성할 값을 작성한다. 
    SetProxyDefine(FuncName.upper())

    # 2. Proxy.h 파일에 작성할 값을 작성한다. 
    SetProxyHeader(DataType, FuncName, Param)
    
    # 3. Proxy.cpp 파일에 작성할 값을 작성한다. 
    SetProxyCpp(DataType, FuncName, Param)

    # 4. Stub.h 파일에 작성할 값을 작성한다. 
    SetStubHeader(DataType, FuncName, Param)

    # 5. Stub.cpp 파일에 작성할 값을 작성한다. 
    SetStubCpp(DataType, FuncName, Param)

    # 6. Stub.cpp 파일에 있는 switch 문을 작성한다.
    SetStubCppSwitch(DataType, FuncName, Param)
    
    PACKETNUM += 1

    

    


def main():

    File = open("./IDL.cnf", 'r', encoding = 'utf-8')
    while True:
        line = File.readline()
        if not line: break 

        # 엔터를 누른 빈 줄이면 건너뛴다.
        if line == '\n':
            continue
        # '//' 로 시작하면 주석이다. 
        elif line[0:2] == "//":
            continue
        # '#' 로 시작하면 내가 등록한 나만의 키워드이다.
        elif line[0] == "#":
            MyKeyWord(line, File)
        # 엔터를 누른 빈 줄이면 건너뛴다.
        else:
            ParsingFunc(line.strip())
            
    File.close()



    

if __name__ == "__main__":
    main()
    SaveFile()
