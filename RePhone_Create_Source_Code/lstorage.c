
#include "vmlog.h"
#include "vmfs.h"
#include "vmchset.h"
#include <stdio.h>
#include <string.h>
#include "lstorage.h"


void file_create(const char* fileName)
{
	VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
	VM_FS_HANDLE filehandle = -1;

	sprintf((char*)filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fileName);
	vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);

	if((filehandle = vm_fs_open(wfilename, VM_FS_MODE_CREATE_ALWAYS_WRITE, TRUE)) < 0)
	{
		vm_log_info("Failed to create file: %s",filename);
		return;
	}
	vm_log_info("Success to create file: %s", filename);
	vm_fs_close(filehandle);
}

void file_open(const char* fileName)
{
	VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
	VM_FS_HANDLE filehandle = -1;

	sprintf((char*)filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fileName);
	vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);

	if((filehandle = vm_fs_open(wfilename, VM_FS_MODE_READ, TRUE)) < 0)
	{
		vm_log_info("Failed to open file: %s",filename);

		if((filehandle = vm_fs_open(wfilename, VM_FS_MODE_CREATE_ALWAYS_WRITE, TRUE)) < 0)
		{
			vm_log_info("Failed to create file: %s",filename);
			return;
		}

		vm_log_info("Success to create file: %s", filename);
		vm_fs_close(filehandle);
		return;
	}

	//vm_log_info("Success to open file: %s", filename);
	vm_fs_close(filehandle);
}

void file_write(const char* fileName, const char* strBuf, long pos)
{
	VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
	VM_FS_HANDLE filehandle = -1;
	VMUINT writelen = 0;
	VMINT ret = 0;

	sprintf((char*)filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fileName);
	vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);

	// write file
	if((filehandle = vm_fs_open(wfilename, VM_FS_MODE_WRITE, TRUE)) < 0)
	{
		vm_log_info("Write failed to open file: %s",filename);
		return;
	}
	vm_log_info("Write success to open file: %s", filename);

	vm_fs_seek(filehandle, pos, VM_FS_BASE_END);

	ret = vm_fs_write(filehandle, (void*)strBuf, strlen(strBuf), &writelen);
	if(ret < 0)
	{
		vm_log_info("Failed to write file");
		return;
	}
	vm_log_info("Success to write file: %s", filename);
	vm_fs_close(filehandle);
}

void file_delete(const char* fileName)
{
	VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMINT ret = 0;

	sprintf((char*)filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fileName);
	vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);

	ret = vm_fs_delete(wfilename);
	if(ret == 0)
	{
		vm_log_info("Success to delete file: %s",filename);
	}
	else
	{
		vm_log_info("Failed to delete file: %s",filename);
	}
}

void file_read(const char* fileName, char* strBuf, unsigned int nByte, long pos)
{
	VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
	VM_FS_HANDLE filehandle = -1;
	VMUINT readlen = 0;
	VMINT ret = 0;

	sprintf((char*)filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fileName);
	vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);

	if((filehandle = vm_fs_open(wfilename, VM_FS_MODE_READ, TRUE)) < 0)
	{
		vm_log_info("Failed to open file: %s",filename);
		return;
	}
	vm_log_info("Success to open file: %s", filename);

	ret = vm_fs_seek(filehandle, pos, VM_FS_BASE_BEGINNING);
	if (ret < 0)
	{
		vm_log_info("Failed to seek the file.");
		vm_fs_close(filehandle);
		return;
	}

	vm_log_info("Success to seek the file.");

	ret = vm_fs_read(filehandle, (void*)strBuf, nByte, &readlen);
	if (ret < 0)
	{
		vm_log_info("Failed to read the file.");
		vm_fs_close(filehandle);
		return;
	}

	vm_log_info("Success to read the file.");
	vm_fs_close(filehandle);
}

void file_size(const char* fileName, unsigned long* size)
{
	VMCHAR filename[VM_FS_MAX_PATH_LENGTH] = {0};
	VMWCHAR wfilename[VM_FS_MAX_PATH_LENGTH] = {0};
	VM_FS_HANDLE filehandle = -1;

	sprintf((char*)filename, "%c:\\%s", vm_fs_get_internal_drive_letter(), fileName);
	vm_chset_ascii_to_ucs2(wfilename, sizeof(wfilename), filename);

	if((filehandle = vm_fs_open(wfilename, VM_FS_MODE_READ, TRUE)) < 0)
	{
		vm_log_info("Failed to open file: %s",filename);
		return;
	}

	//vm_log_info("Success to open file: %s", filename);
	vm_fs_get_size(filehandle, (VMUINT*)size);
	vm_fs_close(filehandle);
}

