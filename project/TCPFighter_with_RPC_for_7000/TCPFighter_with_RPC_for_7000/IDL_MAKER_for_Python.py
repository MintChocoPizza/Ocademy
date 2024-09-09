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
    # ./PacketDefine.h ���� �ۼ� 
    PacketDefineHeader = open("./Protocol.h", "w", encoding="utf-16") 
    PacketDefineHeader.write("#ifndef __PROTOCOL_H__ \n")
    PacketDefineHeader.write("#define __PROTOCOL_H__ \n\n") 
    PacketDefineHeader.write(PacketDefineHeaderStr)
    PacketDefineHeader.write("\n#endif")
    PacketDefineHeader.close() 
    
def WriteProxyHeader():
    global ProxyHeaderStr
    
    ########################################################
    # ./Proxy.h ���� �ۼ� 
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
    # ./Proxy.cpp ���� �ۼ�
    ProxyCpp = open("./Proxy.cpp", "w", encoding="utf-16")

    ########################################################
    # ����ȭ ���� ����
    # Protocol.h ����
    ProxyCpp.write("#include \"Protocol.h\" \n")
    ProxyCpp.write("#include \"SerializeBuffer.h\" \n")
    ProxyCpp.write("#include \"proxy.h\" \n")
    ProxyCpp.write("\n" + DefineDisableWarning + "\n\n")
    ProxyCpp.write(DefineForwardDeclaration + "\n")
    ProxyCpp.write("OreoPizza::Proxy proxy; \n")


    
    ########################################################
    # ��Ʈ��ũ ���̺귯���� ���� �¹��� �־�� ��.
    # But ���� ��Ʈ��ũ ���̺귯���� ��� �ϴ� �ǳ� �ڴ�. 

    ProxyCpp.write("\n")
    ProxyCpp.write(ProxyCppStr)
    ProxyCpp.close()

def WriteStubHeader():
    global StubHeaderStr

    ########################################################
    # ./stup.h ���� �ۼ�
    StubHeader = open("./Stub.h", "w", encoding="utf-16")
    StubHeader.write("#ifndef __STUB_H__ \n")
    StubHeader.write("#define __STUB_H__ \n\n")
    
    StubHeader.write("namespace OreoPizza \n")
    StubHeader.write("{ \n")
    
    StubHeader.write("\tclass Stub \n")
    StubHeader.write("\t{ \n")
    
    # ���� �ۼ��� ������ ����.
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
    # ����ȭ ���� ����
    StubCpp.write("#include \"SerializeBuffer.h\" \n")
    StubCpp.write("#include \"Protocol.h\" \n")
    StubCpp.write("#include \"Stub.h\" \n")
    StubCpp.write("OreoPizza::Stub stub; \n")
    StubCpp.write("\n")
    # ����ġ�� �ۼ�
    WriteStubProcessRecvMessage(StubCpp)

    # ���� �Լ� �ۼ�
    StubCpp.write(StubCppStr)

    StubCpp.close()    

def WriteStubProcessRecvMessage(StubCpp):
    global StubCppSwitchStr
    global DefineTypeValue
    global StubMessageParam

    StubCpp.write("void OreoPizza::Stub::ProcessRecvMessage(" + StubMessageParam + ") \n{ \n")
    StubCpp.write("\tswitch("+ DefineTypeValue +") \n\t{ \n")
    
    # �޽������� PacketType �̾Ƴ���
    #
    
    StubCpp.write(StubCppSwitchStr)

    StubCpp.write("\tdefault: \n")
    # �̻��� Ÿ���� ����ó��~~~~
    StubCpp.write("\t\tbreak; \n")
    StubCpp.write("\t} \n")
    StubCpp.write("} \n\n")

def SaveFile():


    ########################################################
    # ./PacketDefine.h ���� �ۼ� 
    WritePacketDefineHeader()
    
    ########################################################
    # ./Proxy.h ���� �ۼ� 
    WriteProxyHeader()

    ########################################################
    # ./Proxy.cpp ���� �ۼ�
    WriteProxyCpp()

    ########################################################
    # ./stup.h ���� �ۼ�
    WriteStubHeader()

    ########################################################
    # ./stup.cpp ���� �ۼ�
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
    # 1. ����ȭ ���ۿ� ��Ŷ�� ����� �ִ´�. 
    # 2. ����ȭ ���ۿ� �����͸� �ִ´�. 
    # 3. ����ȭ ���ۿ� ����� ��� ������ ��ġ�� �ִ´�.
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
    # ���漱��� �޽��� ���� �Լ�
    # ��Ʈ��ũ �ڵ忡 ���� �Լ� �ۼ��Ǿ� ����
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

    # �Ķ���Ϳ��� Ű���带 �����Ͽ� ������ �Ѵ�.
    ProxyHeaderStr += "(" + "int destID, " + removeKeyWord(Param) + "); \n"

    return

def SetProxyCpp(DataType, FuncName, Param) :
    global ProxyCppStr
    global ProxyCppDefine
    global KeyWordTable
    

    ProxyCppStr += DataType + " " + ProxyCppDefine + FuncName 

    # �Ķ���Ϳ��� Ű���带 �����Ѵ�. 
    ProxyCppStr += "(" + "int DestID, " + removeKeyWord(Param) + ") \n"
        
    ProxyCppStr += "{ \n"

    ########################################################
    # ����ȭ ���ۿ� �ִ´�.
    ProxyCppStr += "\t" + PushBuff(Param) + "\n"
    
    ########################################################
    # Send�� �Ѵ�. 
    

    ProxyCppStr += "} \n\n"
    return 
    
def SetStubHeader(DataType, FuncName, Param) :
    global StubHeaderStr 
    global StubCoverDefine
    global KeyWordTable
    global StubConverParam
    
    # private �� �̷���� �Լ��� �޽����� �޾Ƽ� �޽����� ���� 
    # protected �� �̷���� �����Լ��� �Ķ���ͷ� ���� �޾Ƽ� �˾Ƽ� ó���Ѵ�.
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
    # private ������ �Լ�
    StubCppStr += "void " + StubCppDefine + StubCoverDefine + FuncName + StubConverParam + " \n"
    StubCppStr += "{ \n"

    # �޽����� ���� �Ķ���� ���� ���ڿ� �ۼ�
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

    # ���� ���ڿ��� ���ڿ� �Է�
    StubCppStr += "\n\t" + TempStr + "; \n\n"
    
    # �޽����� �� �����Լ��� �Ķ���ͷ� �ѱ��. 
    StubCppStr += "\t" + FuncName + "(" + TempParamStr +"); \n"

    StubCppStr += "} \n\n"
    


    ########################################################
    # ����ڰ� ���� ����� �Լ� 
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
    

    # 1. PacketDefine.h ���Ͽ� �ۼ��� ���� �ۼ��Ѵ�. 
    SetProxyDefine(FuncName.upper())

    # 2. Proxy.h ���Ͽ� �ۼ��� ���� �ۼ��Ѵ�. 
    SetProxyHeader(DataType, FuncName, Param)
    
    # 3. Proxy.cpp ���Ͽ� �ۼ��� ���� �ۼ��Ѵ�. 
    SetProxyCpp(DataType, FuncName, Param)

    # 4. Stub.h ���Ͽ� �ۼ��� ���� �ۼ��Ѵ�. 
    SetStubHeader(DataType, FuncName, Param)

    # 5. Stub.cpp ���Ͽ� �ۼ��� ���� �ۼ��Ѵ�. 
    SetStubCpp(DataType, FuncName, Param)

    # 6. Stub.cpp ���Ͽ� �ִ� switch ���� �ۼ��Ѵ�.
    SetStubCppSwitch(DataType, FuncName, Param)
    
    PACKETNUM += 1

    

    


def main():

    File = open("./IDL.cnf", 'r', encoding = 'utf-8')
    while True:
        line = File.readline()
        if not line: break 

        # ���͸� ���� �� ���̸� �ǳʶڴ�.
        if line == '\n':
            continue
        # '//' �� �����ϸ� �ּ��̴�. 
        elif line[0:2] == "//":
            continue
        # '#' �� �����ϸ� ���� ����� ������ Ű�����̴�.
        elif line[0] == "#":
            MyKeyWord(line, File)
        # ���͸� ���� �� ���̸� �ǳʶڴ�.
        else:
            ParsingFunc(line.strip())
            
    File.close()



    

if __name__ == "__main__":
    main()
    SaveFile()
