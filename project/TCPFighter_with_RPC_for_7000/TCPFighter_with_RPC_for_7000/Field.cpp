
#include <Windows.h>
#include <list>
#include <unordered_map>
#include "LOG.h"
#include "Protocol.h"
#include "C_Ring_Buffer.h"
#include "SerializeBuffer.h"
#include "main.h"
#include "Disconnect.h"
#include "Session.h"
#include "Define.h"
#include "CList.h"
#include "Field.h"
#include "Player.h"

C_Field C_Field::_C_Field;

//--------------------------------------------
// 현재위치 + 8방
// 현재위치, 좌측 상단부터 우측 하단으로 순서대로
//--------------------------------------------
int dY[] = { 0, -1,-1,-1,  0, 0,  1,1,1 };
int dX[] = { 0, -1, 0, 1, -1, 1, -1,0,1 };




//st_SECTOR_POS::st_SECTOR_POS() 
//{ 
//    iX = NULL;
//    iY = NULL;
//}

//st_SECTOR_POS::st_SECTOR_POS(int X, int Y)
//{
//    iX = X / C_Field::GetInstance()->Grid_X_Size;
//    iY = Y / C_Field::GetInstance()->Grid_Y_Size;
//}


C_Field* C_Field::GetInstance(void)
{
    return &_C_Field;
}

void C_Field::GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND* pSectorAound)
{
    int iCnt;
    int nSectorY;
    int nSectorX; 
    int iCount;

    iCount = 0;

    for (iCnt = 0; iCnt < 9; ++iCnt)
    {
        nSectorY = iSectorY + dY[iCnt];
        nSectorX = iSectorX + dX[iCnt];

        if (Check_Sector_CoordinateRange(nSectorX, nSectorY))
        {
            pSectorAound->Around[iCount].iY = nSectorY;
            pSectorAound->Around[iCount].iX = nSectorX;
            iCount++;
        }
    }

    pSectorAound->iCount = iCount;
}

void C_Field::GetAttackSectorAround(short shX, short shY, char byDirection, int i_Attack_Range_X, int i_Attack_Range_Y, st_SECTOR_AROUND* pSectorAound)
{
    int iCnt;
    int iCurSectorY;
    int iCurSectorX;
    int iSectorY;
    int iSectorX;

    iCurSectorY = shY / dfGRID_Y_SIZE;
    iCurSectorX = shX / dfGRID_X_SIZE;

    //------------------------------------------------------------
    // 자신이 있던 섹터는 무조건 공격 범위 섹터에 포함된다.
    pSectorAound->Around[0].iY = iCurSectorY;
    pSectorAound->Around[0].iX = iCurSectorX;
    iCnt = 1;

    if (byDirection == dfPACKET_MOVE_DIR_LL)
    {
        // 왼쪽 방향 공격

        // 왼쪽 방향 섹터 검색
        iSectorX = (shX - i_Attack_Range_X) / dfGRID_X_SIZE;
        if (Check_Sector_CoordinateRange(iSectorX, iCurSectorY)  && iCurSectorX != iSectorX)
        {
            pSectorAound->Around[iCnt].iX = iSectorX;
            pSectorAound->Around[iCnt].iY = iCurSectorY;
            ++iCnt;

            // 왼쪽 방향 섹터 확정/ 위 아래 섹터 확인
            iSectorY = (shY + i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;

                // 2중 if() 문으로 대각선에 위치한 섹터 구함
                pSectorAound->Around[iCnt].iX = iSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }

            iSectorY = (shY - i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;

                // 2중 if() 문으로 대각선에 위치한 섹터 구함
                pSectorAound->Around[iCnt].iX = iSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }
        }
        else
        {
            // 오른쪽 방향이 다른 섹터로 넘어가지 않는다면, 위 아래 방향의 섹터만 탐색해보면 된다.
            iSectorY = (shY + i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }

            iSectorY = (shY - i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }
        }
    }
    else
    {
        // 오른쪽 방향 공격
        
        // 오른쪽 방향 섹터 검색
        iSectorX = (shX + i_Attack_Range_X) / dfGRID_X_SIZE;
        if (Check_Sector_CoordinateRange(iSectorX, iCurSectorY) && iCurSectorX != iSectorX)
        {
            pSectorAound->Around[iCnt].iX = iSectorX;
            pSectorAound->Around[iCnt].iY = iCurSectorY;
            ++iCnt;

            // 오른쪽 방향 섹터를 탐색해야 하는 상태에서 위쪽 혹은 아랫쪽 섹터도 탐색해야 하는지 확인
            iSectorY = (shY + i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt; 

                // 2중 if() 문으로 대각선에 위치한 섹터 구함
                pSectorAound->Around[iCnt].iX = iSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }

            iSectorY = (shY - i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if(Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;

                // 2중 if() 문으로 대각선에 위치한 섹터 구함
                pSectorAound->Around[iCnt].iX = iSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }
        }
        else
        {
            // 오른쪽 방향이 다른 섹터로 넘어가지 않는다면, 위 아래 방향의 섹터만 탐색해보면 된다.
            iSectorY = (shY + i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }

            iSectorY = (shY - i_Attack_Range_Y) / dfGRID_Y_SIZE;
            if (Check_Sector_CoordinateRange(iCurSectorX, iSectorY) && iCurSectorY != iSectorY)
            {
                pSectorAound->Around[iCnt].iX = iCurSectorX;
                pSectorAound->Around[iCnt].iY = iSectorY;
                ++iCnt;
            }
        }
    }

    pSectorAound->iCount = iCnt;

}

void C_Field::GetUpdateSectorAround(st_PLAYER* pCharacter, st_SECTOR_AROUND* pRemoveSector, st_SECTOR_AROUND* pAddSector)
{
    //--------------------------------------------
       // 8방 이동.
       // But 대각선 이동은, 수평이동 + 수직이동 임.
       // 
       // 나(pPlayer) 빼고 전송해야 한다.
       //--------------------------------------------
    int iRemoveCount = 0;
    int iAddCount = 0;
    st_SECTOR_POS* st_Old_Sector = pCharacter->_OldSector;
    st_SECTOR_POS* st_Cur_Sector = pCharacter->_CurSector;

    _LOG(0, L"# SectorUpdate # SessionID:%d / Old_Sector: X:%d / Y:%d - Cur_Sector: X:%d / Y:%d",pCharacter->_SessionID, st_Old_Sector->iX, st_Old_Sector->iY, st_Cur_Sector->iX, st_Cur_Sector->iY);

    // 위 방향 이동
    if (st_Old_Sector->iY > st_Cur_Sector->iY && st_Old_Sector->iX == st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }   
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 0, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 0;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 0, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 0;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }        
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
    }
    // 아래 방향 이동
    else if (st_Old_Sector->iY < st_Cur_Sector->iY && st_Old_Sector->iX == st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 0, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 0;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 0, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 0;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }

    }
    // 왼쪽 방향 이동
    else if (st_Old_Sector->iY == st_Cur_Sector->iY && st_Old_Sector->iX > st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 0))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 0;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 0))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 0;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
    }
    // 오른쪽 방향 이동
    else if (st_Old_Sector->iY == st_Cur_Sector->iY && st_Old_Sector->iX < st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터 
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 0))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 0;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 0))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 0;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }

    }
    // 좌측 상단
    else if (st_Old_Sector->iY > st_Cur_Sector->iY && st_Old_Sector->iX > st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 0))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 0;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 0, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 0;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 0, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 0;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 0))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 0;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
    }
    // 우측 상단
    else if (st_Old_Sector->iY > st_Cur_Sector->iY && st_Old_Sector->iX < st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 0))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 0;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 0, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 0;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 0, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 0;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 0))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 0;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
    }
    // 좌측 하단 
    else if (st_Old_Sector->iY < st_Cur_Sector->iY && st_Old_Sector->iX > st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 0, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 0;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 0))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 0;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 0))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 0;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 0, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 0;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
    }
    // 우측 하단
    else if (st_Old_Sector->iY < st_Cur_Sector->iY && st_Old_Sector->iX < st_Cur_Sector->iX)
    {
        // 기존 영향권에서 빠지는 섹터 
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX + 0, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX + 0;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY - 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY - 1;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 0))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 0;
            ++iRemoveCount;
        }
        if (Check_Sector_CoordinateRange(st_Old_Sector->iX - 1, st_Old_Sector->iY + 1))
        {
            pRemoveSector->Around[iRemoveCount].iX = st_Old_Sector->iX - 1;
            pRemoveSector->Around[iRemoveCount].iY = st_Old_Sector->iY + 1;
            ++iRemoveCount;
        }

        // 새로 들어온 섹터 영향권 
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY - 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY - 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 0))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 0;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX + 0, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX + 0;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
        if (Check_Sector_CoordinateRange(st_Cur_Sector->iX - 1, st_Cur_Sector->iY + 1))
        {
            pAddSector->Around[iAddCount].iX = st_Cur_Sector->iX - 1;
            pAddSector->Around[iAddCount].iY = st_Cur_Sector->iY + 1;
            ++iAddCount;
        }
    }
    pRemoveSector->iCount = iRemoveCount;
    pAddSector->iCount = iAddCount;

    for (int i = 0; i < pRemoveSector->iCount; ++i)
    {
        if (pRemoveSector->Around[i].iX < 0 || 300 < pRemoveSector->Around[i].iX)
            __debugbreak();
        if (pRemoveSector->Around[i].iY < 0 || 300 < pRemoveSector->Around[i].iY)
            __debugbreak();
    }
    for (int i = 0; i < pAddSector->iCount; ++i)
    {
        if (pAddSector->Around[i].iX < 0 || 300 < pAddSector->Around[i].iX)
            __debugbreak();
        if (pAddSector->Around[i].iY < 0 || 300 < pAddSector->Around[i].iY)
            __debugbreak();
    }
}

bool C_Field::Sector_UpdateCharacter(st_PLAYER* pPlayer)
{
    int SectorY;
    int SectorX;

    SectorY = pPlayer->_Y / dfGRID_Y_SIZE;
    SectorX = pPlayer->_X / dfGRID_X_SIZE;

    // 실제 섹터와 저장되어 있는 섹터가 다르다면, 갱신한다. 
    if (pPlayer->_CurSector->iX != SectorX || pPlayer->_CurSector->iY != SectorY)
    {
        //----------------------------------------------------------------
        // 이런식으로 동적할당 된 변수를 '=' 연산하면 문제가 발생함
        // pPlayer->_OldSector = pPlayer->_CurSector;
        pPlayer->_OldSector->iY = pPlayer->_CurSector->iY;
        pPlayer->_OldSector->iX = pPlayer->_CurSector->iX;

        pPlayer->_CurSector->iY = SectorY;
        pPlayer->_CurSector->iX = SectorX;

        g_Sector_CList[pPlayer->_OldSector->iY][pPlayer->_OldSector->iX].remove(pPlayer);
        g_Sector_CList[SectorY][SectorX].push_back(pPlayer);

        return true;
    }

    return false;
}

void C_Field::CharacterSectorUpdatePacket(st_PLAYER* pPlayer)
{
    //--------------------------------------------
    // 8방 이동.
    // But 대각선 이동은, 수평이동 + 수직이동 임.
    // 
    // 나(pPlayer) 빼고 전송해야 한다.
    //--------------------------------------------
    st_SECTOR_AROUND st_RemoveSector_Around;
    st_SECTOR_AROUND st_AddSector_Around;
    CList<st_PLAYER*>::iterator iter;
    int iCnt;

    GetUpdateSectorAround(pPlayer, &st_RemoveSector_Around, &st_AddSector_Around);

    //------------------------------------------------------------------------------------------
    // 기존 영향권에서 빠지는 섹터에 대해서, 서로 삭제 메시지를 보내야 한다.
    // pPlayer의 삭제 메시지를 기존 영향권에서 빠지는 섹터에 보낸다. 
    mpDeleteCharacter(&g_Packet, pPlayer->_SessionID);
    for (iCnt = 0; iCnt < st_RemoveSector_Around.iCount; ++iCnt)
    {
        SendPacket_SectorOne(st_RemoveSector_Around.Around[iCnt].iX, st_RemoveSector_Around.Around[iCnt].iY, &g_Packet, pPlayer->_pSession);
    }
    g_Packet.Clear();
    // pPlayer에게 기존 영향권에서 빠지는 섹터의 플레이어의 삭제 메시지를 보낸다.
    SendPacket_Around_To_Session_DeleteCharater(pPlayer->_pSession, &g_Packet, &st_RemoveSector_Around);


    //------------------------------------------------------------------------------------------
    // 새로 들어온 영향권에 대해서 서로 생성 메시지를 보내야 한다.
    // 또한 액션을 취하는 중이라면, 해당 동작 또한 이어서 보내야 한다. 
    mpCreateOtherCharacter(&g_Packet, pPlayer->_SessionID, pPlayer->_byDirection, pPlayer->_X, pPlayer->_Y, pPlayer->_HP);
    for (iCnt = 0; iCnt < st_AddSector_Around.iCount; ++iCnt)
    {
        SendPacket_SectorOne(st_AddSector_Around.Around[iCnt].iX, st_AddSector_Around.Around[iCnt].iY, &g_Packet, pPlayer->_pSession);
    }
    g_Packet.Clear();
    // pPlayer가 액션 중이라면 해당 액션을 이어서 보여줘야한다. 
    switch (pPlayer->_dwAction)
    {
    case dfPACKET_MOVE_DIR_LL:
    case dfPACKET_MOVE_DIR_LU:
    case dfPACKET_MOVE_DIR_UU:
    case dfPACKET_MOVE_DIR_RU:
    case dfPACKET_MOVE_DIR_RR:
    case dfPACKET_MOVE_DIR_RD:
    case dfPACKET_MOVE_DIR_DD:
    case dfPACKET_MOVE_DIR_LD:
        mpMoveStart(&g_Packet, pPlayer->_SessionID, pPlayer->_byMoveDirection, pPlayer->_X, pPlayer->_Y);
        for (iCnt = 0; iCnt < st_AddSector_Around.iCount; ++iCnt)
        {
            SendPacket_SectorOne(st_AddSector_Around.Around[iCnt].iX, st_AddSector_Around.Around[iCnt].iY, &g_Packet, pPlayer->_pSession);
        }
        g_Packet.Clear();
        break;
    case dfPACKET_CS_MOVE_STOP:
        break;

    default:
        // 이상함 유저의 연결을 끊는다.
        //_LOG(dfLOG_LEVEL_SYSTEM, L"# CharacterSectorUpdatePacket # error SessionID:%d", pPlayer->_SessionID);
        LOG(L"Disconnect", LEVEL_SYSTEM, false, true, L"# CharacterSectorUpdatePacket # error SessionID:%d", pPlayer->_SessionID);
        enqueueForDeletion(pPlayer->_SessionID);
        break;
    }

    // 주변 섹터의 플레이어들의 정보를 pPlayer에게 전달해준다.
    SendPacket_Around_To_Session_CreateCharacter(pPlayer->_pSession, &g_Packet, &st_AddSector_Around);
}

void C_Field::removeUserFromSector(st_PLAYER* pPlayer)
{
    g_Sector_CList[pPlayer->_CurSector->iY][pPlayer->_CurSector->iX].remove(pPlayer);
}

void C_Field::SendPacket_SectorOne(int iSectorX, int iSectorY, SerializeBuffer* pPacket, st_SESSION* pExceptSession)
{
    //std::unordered_map<DWORD, st_PLAYER*> ::iterator iter;
    CList<st_PLAYER*>::iterator iter;

    if (pExceptSession == NULL)
    {
        for (iter = g_Sector_CList[iSectorY][iSectorX].begin(); iter != g_Sector_CList[iSectorY][iSectorX].end(); ++iter)
        {
            SendPacket_Unicast((*iter)->_pSession, pPacket);
        }
    }
    else
    {
        for (iter = g_Sector_CList[iSectorY][iSectorX].begin(); iter != g_Sector_CList[iSectorY][iSectorX].end(); ++iter)
        {
            if (pExceptSession == (*iter)->_pSession)
            {
                continue;
            }

            SendPacket_Unicast((*iter)->_pSession, pPacket);
        }
    }
}

void C_Field::SendPacket_Around(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around, bool bSendMe)
{
    int iCnt;

    if (bSendMe == false)
    {
        for (iCnt = 0; iCnt < pSector_Around->iCount; ++iCnt)
        {
            SendPacket_SectorOne(pSector_Around->Around[iCnt].iX, pSector_Around->Around[iCnt].iY, pPacket, pSession);
        }
    }
    else
    {
        for (iCnt = 0; iCnt < pSector_Around->iCount; ++iCnt)
        {
            SendPacket_SectorOne(pSector_Around->Around[iCnt].iX, pSector_Around->Around[iCnt].iY, pPacket, NULL);
        }
    }
}

void C_Field::SendPacket_Around_To_Session_CreateCharacter(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around)
{
    int iCnt;
    CList<st_PLAYER*>::iterator iter;
    st_PLAYER* pPlayer;

    for (iCnt = 0; iCnt < pSector_Around->iCount; ++iCnt)
    {
        for(iter = g_Sector_CList[pSector_Around->Around[iCnt].iY][pSector_Around->Around[iCnt].iX].begin(); iter != g_Sector_CList[pSector_Around->Around[iCnt].iY][pSector_Around->Around[iCnt].iX].end(); ++iter)
        {
            pPlayer = *iter;
            if (pPlayer->_pSession == pSession) continue;

            mpCreateOtherCharacter(pPacket, pPlayer->_SessionID, pPlayer->_byDirection, pPlayer->_X, pPlayer->_Y, pPlayer->_HP);
            SendPacket_Unicast(pSession, pPacket);
            pPacket->Clear();

			//----------------------------------------------------------------------------------
			// pSession에 pPlayer생성 메시지를 보내야 한다. 움직이고 있다면, 움직이는것도
			switch (pPlayer->_dwAction)
			{
			case dfPACKET_MOVE_DIR_LL:
			case dfPACKET_MOVE_DIR_LU:
			case dfPACKET_MOVE_DIR_UU:
			case dfPACKET_MOVE_DIR_RU:
			case dfPACKET_MOVE_DIR_RR:
			case dfPACKET_MOVE_DIR_RD:
			case dfPACKET_MOVE_DIR_DD:
			case dfPACKET_MOVE_DIR_LD:
                mpMoveStart(pPacket, pPlayer->_SessionID, pPlayer->_byMoveDirection, pPlayer->_X, pPlayer->_Y);
                SendPacket_Unicast(pSession, pPacket);
                pPacket->Clear();
                break;
			case dfPACKET_CS_MOVE_STOP:
				break;

			default:
				// 이상함 유저의 연결을 끊는다.
                //_LOG(dfLOG_LEVEL_SYSTEM, L"#SendPacket_Around_To_Session_CreateCharacter Error");
                LOG(L"Disconnect", LEVEL_SYSTEM, false, true, L"#SendPacket_Around_To_Session_CreateCharacter Error");
                enqueueForDeletion(pPlayer->_SessionID);
				break;
			}
        }
    }
}

void C_Field::SendPacket_Around_To_Session_DeleteCharater(st_SESSION* pSession, SerializeBuffer* pPacket, st_SECTOR_AROUND* pSector_Around)
{
    int iCnt;
    CList<st_PLAYER*>::iterator iter;
    st_PLAYER* pPlayer;

    for (iCnt = 0; iCnt < pSector_Around->iCount; ++iCnt)
    {
        for(iter = g_Sector_CList[pSector_Around->Around[iCnt].iY][pSector_Around->Around[iCnt].iX].begin(); iter != g_Sector_CList[pSector_Around->Around[iCnt].iY][pSector_Around->Around[iCnt].iX].end(); ++iter)
        {
            pPlayer = *iter;

            mpDeleteCharacter(pPacket, pPlayer->_SessionID);
            SendPacket_Unicast(pSession, pPacket);
            pPacket->Clear();
        }
    }

}

bool C_Field::Check_Sector_CoordinateRange(int iSectorX, int iSectorY)
{
    if (iSectorX < 0 || iSectorY < 0 || iSectorX >= dfSECTOR_MAX_X || iSectorY >= dfSECTOR_MAX_Y)
        return false;
    return true;
}

CList<st_PLAYER*>* C_Field::GetPlayerInSectorCList(int iSectorX, int iSectorY)
{
    return &g_Sector_CList[iSectorY][iSectorX];
}

void st_SECTOR_POS::Init_SECTOR_POS(int Y, int X)
{
    iY = Y / (dfGRID_Y_SIZE);
    iX = X / (dfGRID_X_SIZE);
}

void C_Field::AddPlayerToSector(st_PLAYER* pPlayer)
{
    g_Sector_CList[pPlayer->_CurSector->iY][pPlayer->_CurSector->iX].push_back(pPlayer);
}

C_Field::C_Field() : _MemPool(8001), g_Sector_CList()
{
    int iCntY;
    int iCntX;

    //_Sector_Max_Y = Sector_Max_Y = dfRANGE_MOVE_BOTTOM / dfGRID_Y_SIZE;
    //_Sector_Max_X = Sector_Max_X = dfRANGE_MOVE_RIGHT / dfGRID_X_SIZE;

    for (iCntY = 0; iCntY < dfSECTOR_MAX_Y; ++iCntY)
    {
        for (iCntX = 0; iCntX < dfSECTOR_MAX_X; ++iCntX)
        {
            g_Sector_CList[iCntY][iCntX].InitCList(_MemPool);
        }
    }


    // 3차원 배열: 면, 행, 열 순서
    // 2차원 배열: 행, 열 순서
    // 1차원 배열: 열 순서
    //_Sector = new std::list<st_PLAYER*>**[Sector_Max_Y];
    //for (iCntY = 0; iCntY < Sector_Max_Y; ++iCntY)
    //{
    //    _Sector[iCntY] = new std::list<st_PLAYER*>*[Sector_Max_X];
    //    for (iCntX = 0; iCntX < Sector_Max_X; ++iCntX)
    //    {
    //        _Sector[iCntY][iCntX] = new std::list<st_PLAYER*>();
    //    }
    //}
}

C_Field::~C_Field()
{
    int iCntY;
    int iCntX;
    CList<st_PLAYER*>::iterator iter;

    // 1차 해지 == 메모리풀 메모리 반환하기
    // 물론 또 CList 소멸자가 호출되지만, 상관 없다. 
    for (iCntY = 0; iCntY < dfSECTOR_MAX_Y; ++iCntY)
    {
        for (iCntX = 0; iCntX < dfSECTOR_MAX_X; ++iCntX)
        {
            for (iter = g_Sector_CList[iCntY][iCntX].begin(); iter != g_Sector_CList[iCntY][iCntX].end(); )
            {
                iter = g_Sector_CList[iCntY][iCntX].erase(iter);
            }
        }
    }
}



