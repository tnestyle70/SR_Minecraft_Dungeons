#include "pch.h"
#include "CBlockPreset.h"

void CBlockPreset::Place_OakTree(int ox, int oy, int oz)
{
    auto* pMgr = CBlockMgr::GetInstance();

    // 기둥 (5칸)
    pMgr->AddBlock(ox, oy + 0, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 1, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 2, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 3, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 4, oz, BLOCK_OAK);

    // 잎 y+3: 5x5 (모서리 제거)
    for (int dx = -2; dx <= 2; ++dx)
        for (int dz = -2; dz <= 2; ++dz)
            if (!(abs(dx) == 2 && abs(dz) == 2))
                pMgr->AddBlock(ox + dx, oy + 3, oz + dz, BLOCK_OAK_LEAVES);

    // 잎 y+4: 5x5 (모서리 제거)
    for (int dx = -2; dx <= 2; ++dx)
        for (int dz = -2; dz <= 2; ++dz)
            if (!(abs(dx) == 2 && abs(dz) == 2))
                pMgr->AddBlock(ox + dx, oy + 4, oz + dz, BLOCK_OAK_LEAVES);

    // 잎 y+5: 3x3
    for (int dx = -1; dx <= 1; ++dx)
        for (int dz = -1; dz <= 1; ++dz)
            pMgr->AddBlock(ox + dx, oy + 5, oz + dz, BLOCK_OAK_LEAVES);

    // 꼭대기 십자
    pMgr->AddBlock(ox, oy + 6, oz, BLOCK_OAK_LEAVES);
    pMgr->AddBlock(ox + 1, oy + 6, oz, BLOCK_OAK_LEAVES);
    pMgr->AddBlock(ox - 1, oy + 6, oz, BLOCK_OAK_LEAVES);
    pMgr->AddBlock(ox, oy + 6, oz + 1, BLOCK_OAK_LEAVES);
    pMgr->AddBlock(ox, oy + 6, oz - 1, BLOCK_OAK_LEAVES);
}

void CBlockPreset::Place_CherryTree(int ox, int oy, int oz)
{
    auto* pMgr = CBlockMgr::GetInstance();

    // 기둥 (5칸)
    pMgr->AddBlock(ox, oy + 0, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 1, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 2, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 3, oz, BLOCK_OAK);
    pMgr->AddBlock(ox, oy + 4, oz, BLOCK_OAK);

    // 잎 y+3: 5x5 (모서리 제거)
    for (int dx = -2; dx <= 2; ++dx)
        for (int dz = -2; dz <= 2; ++dz)
            if (!(abs(dx) == 2 && abs(dz) == 2))
                pMgr->AddBlock(ox + dx, oy + 3, oz + dz, BLOCK_CHERRY_LEAVES);

    // 잎 y+4: 5x5 (모서리 제거)
    for (int dx = -2; dx <= 2; ++dx)
        for (int dz = -2; dz <= 2; ++dz)
            if (!(abs(dx) == 2 && abs(dz) == 2))
                pMgr->AddBlock(ox + dx, oy + 4, oz + dz, BLOCK_CHERRY_LEAVES);

    // 잎 y+5: 3x3
    for (int dx = -1; dx <= 1; ++dx)
        for (int dz = -1; dz <= 1; ++dz)
            pMgr->AddBlock(ox + dx, oy + 5, oz + dz, BLOCK_CHERRY_LEAVES);

    // 꼭대기 십자
    pMgr->AddBlock(ox, oy + 6, oz, BLOCK_CHERRY_LEAVES);
    pMgr->AddBlock(ox + 1, oy + 6, oz, BLOCK_CHERRY_LEAVES);
    pMgr->AddBlock(ox - 1, oy + 6, oz, BLOCK_CHERRY_LEAVES);
    pMgr->AddBlock(ox, oy + 6, oz + 1, BLOCK_CHERRY_LEAVES);
    pMgr->AddBlock(ox, oy + 6, oz - 1, BLOCK_CHERRY_LEAVES);
}

void CBlockPreset::Place_Dragon(int ox, int oy, int oz)
{
    auto* pMgr = CBlockMgr::GetInstance();

    auto Add = [&](int x, int y, int z)
        {
            pMgr->AddBlock(x, y, z, BLOCK_OBSIDIAN);
        };

    // =========================================================
    // 1. 몸통 (더 길고 입체적)
    // 중앙 척추
    for (int dz = 0; dz <= 7; ++dz)
        Add(ox, oy + 3, oz + dz);

    // 몸통 두께
    for (int dz = 1; dz <= 6; ++dz)
    {
        Add(ox + 1, oy + 3, oz + dz);
        Add(ox - 1, oy + 3, oz + dz);

        Add(ox, oy + 4, oz + dz);

        if (dz >= 2 && dz <= 5)
        {
            Add(ox + 1, oy + 4, oz + dz);
            Add(ox - 1, oy + 4, oz + dz);
        }
    }

    // 몸통 아래쪽 볼륨
    for (int dz = 2; dz <= 5; ++dz)
        Add(ox, oy + 2, oz + dz);

    // =========================================================
    // 2. 목
    Add(ox, oy + 4, oz + 7);
    Add(ox, oy + 5, oz + 8);
    Add(ox, oy + 5, oz + 9);
    Add(ox, oy + 6, oz + 10);

    // 목 옆 두께
    Add(ox + 1, oy + 5, oz + 9);
    Add(ox - 1, oy + 5, oz + 9);

    // =========================================================
    // 3. 머리
    // 머리 중심
    Add(ox, oy + 6, oz + 11);
    Add(ox, oy + 7, oz + 11);
    Add(ox, oy + 6, oz + 12);

    // 머리 좌우
    Add(ox + 1, oy + 6, oz + 11);
    Add(ox - 1, oy + 6, oz + 11);
    Add(ox + 1, oy + 6, oz + 12);
    Add(ox - 1, oy + 6, oz + 12);

    // 머리 윗부분
    Add(ox + 1, oy + 7, oz + 11);
    Add(ox - 1, oy + 7, oz + 11);

    // 주둥이
    Add(ox, oy + 5, oz + 13);
    Add(ox + 1, oy + 5, oz + 13);
    Add(ox - 1, oy + 5, oz + 13);
    Add(ox, oy + 6, oz + 13);

    // 뿔
    Add(ox + 1, oy + 8, oz + 11);
    Add(ox - 1, oy + 8, oz + 11);
    Add(ox + 2, oy + 8, oz + 10);
    Add(ox - 2, oy + 8, oz + 10);

    // 턱 느낌
    Add(ox, oy + 4, oz + 12);

    // =========================================================
    // 4. 꼬리 (더 길고 점점 내려감)
    Add(ox, oy + 3, oz - 1);
    Add(ox, oy + 3, oz - 2);
    Add(ox, oy + 2, oz - 3);
    Add(ox, oy + 2, oz - 4);
    Add(ox, oy + 2, oz - 5);
    Add(ox, oy + 1, oz - 6);
    Add(ox, oy + 1, oz - 7);
    Add(ox, oy + 1, oz - 8);
    Add(ox, oy + 0, oz - 9);
    Add(ox, oy + 0, oz - 10);

    // 꼬리 끝 갈라짐
    Add(ox + 1, oy + 0, oz - 11);
    Add(ox - 1, oy + 0, oz - 11);

    // 꼬리 중간 가시 느낌
    Add(ox, oy + 4, oz - 1);
    Add(ox, oy + 3, oz - 4);
    Add(ox, oy + 2, oz - 7);

    // =========================================================
    // 5. 오른쪽 날개
    // 날개 뼈대
    Add(ox + 2, oy + 5, oz + 3);
    Add(ox + 4, oy + 6, oz + 2);
    Add(ox + 6, oy + 6, oz + 1);
    Add(ox + 8, oy + 5, oz + 0);
    Add(ox + 10, oy + 4, oz - 1);
    Add(ox + 12, oy + 3, oz - 2);

    // 날개 윗막
    for (int dx = 2; dx <= 5; ++dx)
        for (int dz = 1; dz <= 4; ++dz)
            Add(ox + dx, oy + 5, oz + dz - 1);

    for (int dx = 4; dx <= 8; ++dx)
        for (int dz = 0; dz <= 2; ++dz)
            Add(ox + dx, oy + 4, oz + dz - 1);

    for (int dx = 7; dx <= 11; ++dx)
        for (int dz = -1; dz <= 1; ++dz)
            Add(ox + dx, oy + 3, oz + dz - 1);

    // 날개 아래 굽힘
    Add(ox + 5, oy + 3, oz + 2);
    Add(ox + 6, oy + 2, oz + 2);
    Add(ox + 7, oy + 2, oz + 1);
    Add(ox + 8, oy + 1, oz + 1);
    Add(ox + 9, oy + 1, oz + 0);
    Add(ox + 10, oy + 1, oz + 0);

    // =========================================================
    // 6. 왼쪽 날개 (미러)
    Add(ox - 2, oy + 5, oz + 3);
    Add(ox - 4, oy + 6, oz + 2);
    Add(ox - 6, oy + 6, oz + 1);
    Add(ox - 8, oy + 5, oz + 0);
    Add(ox - 10, oy + 4, oz - 1);
    Add(ox - 12, oy + 3, oz - 2);

    for (int dx = 2; dx <= 5; ++dx)
        for (int dz = 1; dz <= 4; ++dz)
            Add(ox - dx, oy + 5, oz + dz - 1);

    for (int dx = 4; dx <= 8; ++dx)
        for (int dz = 0; dz <= 2; ++dz)
            Add(ox - dx, oy + 4, oz + dz - 1);

    for (int dx = 7; dx <= 11; ++dx)
        for (int dz = -1; dz <= 1; ++dz)
            Add(ox - dx, oy + 3, oz + dz - 1);

    Add(ox - 5, oy + 3, oz + 2);
    Add(ox - 6, oy + 2, oz + 2);
    Add(ox - 7, oy + 2, oz + 1);
    Add(ox - 8, oy + 1, oz + 1);
    Add(ox - 9, oy + 1, oz + 0);
    Add(ox - 10, oy + 1, oz + 0);

    // =========================================================
    // 7. 앞다리
    Add(ox + 1, oy + 2, oz + 5);
    Add(ox + 1, oy + 1, oz + 5);
    Add(ox + 1, oy + 0, oz + 5);

    Add(ox - 1, oy + 2, oz + 5);
    Add(ox - 1, oy + 1, oz + 5);
    Add(ox - 1, oy + 0, oz + 5);

    // 발톱 느낌
    Add(ox + 2, oy + 0, oz + 6);
    Add(ox - 2, oy + 0, oz + 6);

    // =========================================================
    // 8. 뒷다리
    Add(ox + 1, oy + 2, oz + 1);
    Add(ox + 1, oy + 1, oz + 1);
    Add(ox + 1, oy + 0, oz + 1);

    Add(ox - 1, oy + 2, oz + 1);
    Add(ox - 1, oy + 1, oz + 1);
    Add(ox - 1, oy + 0, oz + 1);

    Add(ox + 2, oy + 0, oz + 0);
    Add(ox - 2, oy + 0, oz + 0);

    // =========================================================
    // 9. 등 가시 (엔더드래곤 느낌 강화)
    Add(ox, oy + 5, oz + 1);
    Add(ox, oy + 6, oz + 3);
    Add(ox, oy + 6, oz + 5);
    Add(ox, oy + 5, oz + 7);
}
