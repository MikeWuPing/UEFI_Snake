#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    CHAR8 TestStr[] = "\x80\x81\x82";
    UINTN i;

    Print(L"Debug Chinese Character Values:\n");
    Print(L"TestStr = \"\\x80\\x81\\x82\"\n\n");

    for (i = 0; i < 3; i++) {
        CHAR8 c = TestStr[i];
        UINT8 uc = (UINT8)c;
        INT8 sc = c;  // Signed interpretation

        Print(L"Byte %d: CHAR8 = 0x%02x, (UINT8) = %d, (INT8) = %d\n",
              i, (UINT8)c, uc, (INTN)sc);
    }

    Print(L"\nChecking if 0x80 is recognized as Chinese:\n");
    CHAR8 c = TestStr[0];
    if ((UINT8)c >= 0x80 && (UINT8)c <= 0xA4) {
        Print(L"  YES: (UINT8)0x%02x = %d, Index = %d\n",
              (UINT8)c, (UINT8)c, (UINT8)c - 0x80);
    } else {
        Print(L"  NO\n");
    }

    Print(L"\nPress any key to exit...\n");
    WaitForSingleEvent(gST->ConIn->WaitForKey, 0);
    return EFI_SUCCESS;
}
