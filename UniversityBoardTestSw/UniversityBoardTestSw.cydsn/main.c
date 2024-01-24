/* ========================================
 * This is program to test Cypress University Board.
 * The board contains:
 * 8 LEDs connected to Port 0
 * 6 switches connected to Port 1 bits 0..5
 * This test program performs LEDs blinking in "chess order".
 * Display shows two lines: 
 *     the first line shows target position of swithes.
 *     the second line shows current switches positions.
 * You need to change switches positions to match them to the target value in line 1.
 * You shall do this in two stages.
 * After second stage test completed and you should see "PASS" on the display.
 *
 * Author: Spartak Mankovskyy. 
 *
 * ========================================
*/
#include "project.h"

#define SWITCHES_COUNT      (6) 
#define SWITCH_SIGN_OFF     ('0')
#define SWITCH_SIGN_ON      ('|')

#define SWITCH_TARGET_1     (0x2A)
#define SWITCH_TARGET_2     (0x15)

typedef enum {
    SWITCHES_TARGET_STAGE_INITIAL,
    SWITCHES_TARGET_STAGE_SECOND
} SwitchesTargetEnumType;

typedef enum {
    SM_STATE_IDLE,
    SM_STATE_SWITCH_POS1,
    SM_STATE_SWITCH_POS2,
    SM_STATE_COMPLETED    
} SmStateEnumType; 

char CurrentSwitchesPositionMirror[SWITCHES_COUNT + 1] = {0};    
char TargetSwitchesPositionMirror[SWITCHES_COUNT + 1] = {0}; 

SmStateEnumType        SmState;
uint8 SwitchesPositionsCurrentTarget;
uint8 CurrentSwitchPositions;
uint8 LedsState = 0x55;

void ReportError(void)
{
    LCD_Char_ClearDisplay();
    LCD_Char_Position(0,0);
    LCD_Char_PrintString("ERROR!");
    while (1)
    {
        /* Error loop */
    }
};

uint8 GetCurrentSwitchesTarget(SwitchesTargetEnumType Stage)
{
    uint8 Ret = 0;
    uint8 MatchBitsCount1 = 0;
    uint8 MatchBitsCount2 = 0;
    uint8 Foo;
    uint8 Idx;
    
    if (SWITCHES_TARGET_STAGE_SECOND == Stage)
    {
        Foo = (1 << SWITCHES_COUNT) - 1;
        Ret = (SwitchesPositionsCurrentTarget ^ 0xFF) & Foo;
    }
    else if (SWITCHES_TARGET_STAGE_INITIAL == Stage)
    {
        CurrentSwitchPositions = SwitchesBus_Read();
        Foo = CurrentSwitchPositions ^ SWITCH_TARGET_1;
        for (Idx = 0; Idx < SWITCHES_COUNT; Idx++)
        {
            MatchBitsCount1 = MatchBitsCount1 + (Foo & 0x01);
            Foo = Foo >> 1;
        }
        Foo = CurrentSwitchPositions ^ SWITCH_TARGET_2;
        for (Idx = 0; Idx < SWITCHES_COUNT; Idx++)
        {
            MatchBitsCount2 = MatchBitsCount2 + (Foo & 0x01);
            Foo = Foo >> 1;
        }
        if (MatchBitsCount1 < MatchBitsCount2)
        {
            Ret = SWITCH_TARGET_1; 
        }
        else
        {
            Ret = SWITCH_TARGET_2; 
        }
    }
    else 
    {
        ReportError();
    }   
    
    return Ret;
};

void SwitchTargetBin2String(uint8 BinTarget, char * CharTarget)
{
    uint8 Idx;
    for (Idx = 0; Idx < SWITCHES_COUNT; Idx++)
    {
        if (BinTarget & 0x01)
        {
            *CharTarget = SWITCH_SIGN_ON;
        }
        else
        {
            *CharTarget = SWITCH_SIGN_OFF;
        }
        BinTarget = BinTarget >> 1;
        CharTarget++;
    }
    *CharTarget = 0;
}

void UpdateDisplayInfo(void)
{    
    CurrentSwitchPositions = SwitchesBus_Read();
    SwitchTargetBin2String(SwitchesPositionsCurrentTarget, TargetSwitchesPositionMirror);
    SwitchTargetBin2String(CurrentSwitchPositions, CurrentSwitchesPositionMirror);
    LCD_Char_ClearDisplay();
    LCD_Char_Position(0,0);
    LCD_Char_PrintString(TargetSwitchesPositionMirror);
    LCD_Char_Position(0,SWITCHES_COUNT);
    LCD_Char_PrintString("-target");
    LCD_Char_Position(1,0);
    LCD_Char_PrintString(CurrentSwitchesPositionMirror);
    LCD_Char_Position(1,SWITCHES_COUNT);
    LCD_Char_PrintString("-current");
}

int main(void)
{
    uint8 AnimationIdx = 0;
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    LCD_Char_Init();
    
    SmState = SM_STATE_IDLE;

    for(;;)
    {
        /* Place your application code here. */
        switch (SmState) {
            case SM_STATE_IDLE:
                SmState = SM_STATE_SWITCH_POS1;
                SwitchesPositionsCurrentTarget = 
                    GetCurrentSwitchesTarget(SWITCHES_TARGET_STAGE_INITIAL);  
               UpdateDisplayInfo();
            break;
            case SM_STATE_SWITCH_POS1:
                UpdateDisplayInfo();
                CurrentSwitchPositions = SwitchesBus_Read();
                if (0 == (CurrentSwitchPositions ^ SwitchesPositionsCurrentTarget))
                {
                    SwitchesPositionsCurrentTarget = 
                        GetCurrentSwitchesTarget(SWITCHES_TARGET_STAGE_SECOND);  
                    SmState = SM_STATE_SWITCH_POS2;
                }                
            break;
            case SM_STATE_SWITCH_POS2:
                UpdateDisplayInfo();
                CurrentSwitchPositions = SwitchesBus_Read();
                if (0 == (CurrentSwitchPositions ^ SwitchesPositionsCurrentTarget))
                {  
                    SmState = SM_STATE_COMPLETED;
                }                
            break;
            case SM_STATE_COMPLETED:
                LCD_Char_ClearDisplay();
                LCD_Char_Position(0,AnimationIdx);
                LCD_Char_PrintString("PASS");
            break;            
            default:
            break;
        } // switch case      
        
        if (AnimationIdx > 4)
        {
            AnimationIdx = 0;
            LedsState = LedsState ^ 0xFF;
            LedsBus_Write(LedsState);
        }        
        AnimationIdx++;   
        
        CyDelay(250);

    }
}

/* [] END OF FILE */
