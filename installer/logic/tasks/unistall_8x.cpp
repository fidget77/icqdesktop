#include "stdafx.h"
#include "unistall_8x.h"
#include "../../utils/links/shell_link.h"

namespace installer
{
    namespace logic
    {
        installer::error delete_8x_links(
            const wchar_t* _icq_folder, 
            const wchar_t* _icq_executable_name,  
            const wchar_t* _app_shortcut_name)
        {
            ::CoInitializeEx(0, COINIT_MULTITHREADED);

            installer::error err;

            CAtlString icq_exe_short = _icq_executable_name;

            CAtlString program_dir, start, desktop, quick_launch;

            // get special folders
            {
                if (!::SHGetSpecialFolderPath(0, program_dir.GetBuffer(4096), CSIDL_PROGRAMS, 0))
                    return installer::error(errorcode::get_special_folder);

                program_dir.ReleaseBuffer();
                program_dir += CAtlString(L"\\") + _icq_folder;

                if (!::SHGetSpecialFolderPath(0, start.GetBuffer(4096), CSIDL_STARTMENU, 0))
                    return installer::error(errorcode::get_special_folder);
                start.ReleaseBuffer();

                if (!::SHGetSpecialFolderPath(0, desktop.GetBuffer(4096), CSIDL_DESKTOPDIRECTORY, 0))
                    return installer::error(errorcode::get_special_folder);
                desktop.ReleaseBuffer();

                quick_launch = links::GetQuickLaunchDir();
            }

            links::RemoveFromMFUList(icq_exe_short);

            CAtlString str = program_dir + L"\\" + _app_shortcut_name;
            links::RemoveLink(str);

            str = program_dir + L"\\Uninstall " + _app_shortcut_name;
            links::RemoveLink(str);

            str = program_dir + L"\\Удалить " + _app_shortcut_name;
            links::RemoveLink(str);

            str = start + L"\\" + _app_shortcut_name;
            links::RemoveLink(str);

            str = links::GetQuickLaunchDir();
            str += CAtlString(L"\\") + _app_shortcut_name;
            links::RemoveLink(str);

            str = desktop;
            str += CAtlString(L"\\") + _app_shortcut_name;
            links::RemoveLink(str);

            ::CoUninitialize();

            return err;
        }

        installer::error delete_8x_links()
        {
            if (build::is_icq())
            {
                return delete_8x_links(L"ICQ", L"icq.exe", L"ICQ.lnk");
            }
            else
            {
                delete_8x_links(L"Mail.Ru", L"magent.exe", L"Mail.Ru Agent.lnk");
                delete_8x_links(L"Mail.Ru", L"magent.exe", L"Mail.Ru Агент.lnk");

                return installer::error();
            }


        }

        installer::error delete_8x_registry_and_files()
        {
            const wchar_t* icq_mra_key = (build::is_icq() ? L"Software\\ICQ\\ICQ" : L"Software\\Mail.Ru\\Agent");
            const wchar_t* reg_version = L"version";
            const wchar_t* reg_for_all_users = L"ForAllUsers";
            const wchar_t* reg_install_path = L"InstallPath";
            const wchar_t* mra_uninstall_key = (build::is_icq() ? L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ICQ" : L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MRA");
            const wchar_t* classes_magent = (build::is_icq() ? L"Software\\Classes\\icq" : L"Software\\Classes\\magent");
            const wchar_t* autostart_key = (build::is_icq() ? L"icq" : L"MAgent");
            const wchar_t* icq_profile_folder = (build::is_icq() ? L"ICQ-Profile" : L"Mra");

            CAtlString installed_path;

            CRegKey key_icq;
            if (ERROR_SUCCESS == key_icq.Open(HKEY_CURRENT_USER, icq_mra_key, KEY_READ))
            {
                DWORD len = 4096;
                if (ERROR_SUCCESS == key_icq.QueryStringValue(reg_install_path, installed_path.GetBuffer(len), &len))
                {
                    installed_path.ReleaseBuffer();
                }
            }

            ::SHDeleteValue(HKEY_CURRENT_USER, icq_mra_key, reg_version);
            ::SHDeleteValue(HKEY_CURRENT_USER, icq_mra_key, reg_for_all_users);
            ::SHDeleteValue(HKEY_CURRENT_USER, icq_mra_key, reg_install_path);

            ::SHDeleteKey(HKEY_CURRENT_USER, mra_uninstall_key);

            ::SHDeleteKey(HKEY_CURRENT_USER, icq_mra_key);

            //::SHDeleteKey(HKEY_CURRENT_USER, classes_magent);

            ::SHDeleteValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", autostart_key);

            CAtlString icq_data;
            if (::SHGetSpecialFolderPath(NULL, icq_data.GetBuffer(4096), CSIDL_APPDATA, 1))
            {
                icq_data.ReleaseBuffer();
                icq_data += CAtlString(L"\\") + icq_profile_folder;

                QDir dir_profile(QString::fromUtf16((const ushort*)(const wchar_t*) icq_data));
                dir_profile.removeRecursively();
            }

            if (!installed_path.IsEmpty())
            {
                QString q_installed_path(QString::fromUtf16((const ushort*)(const wchar_t*) installed_path));
                if (build::is_icq())
                {
                    QDir dir_profile(q_installed_path);
                    dir_profile.removeRecursively();
                }
                else
                {
                    QDir dir_mra(q_installed_path + "\\Mra");
                    dir_mra.removeRecursively();

                    QFile::remove(q_installed_path + "\\libvoip_x86.dll");
                    QFile::remove(q_installed_path + "\\magent.exe");
                    QFile::remove(q_installed_path + "\\magentsetup.exe");
                    QFile::remove(q_installed_path + "\\MRAInplaceViewer.dll");
                    QFile::remove(q_installed_path + "\\sciter32.dll");
                    QFile::remove(q_installed_path + "\\vivo.dll");
                }
            }

            return installer::error();
        }
    }
}
