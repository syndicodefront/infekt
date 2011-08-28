[Files]
Source: "C:\Program Files (x86)\Sherlock Software\InnoTools\Downloader\itdownload.dll"; Flags: dontcopy; DestDir: {tmp}

[Code]
(*
modified by cxxjoe 2011
*)

(*
 Inno Tools Downloader DLL
 Copyright (C) Sherlock Software 2008
 Version 0.3.5

 Contact:
  The author, Nicholas Sherlock, at nick@sherlocksoftware.org.
  Comments, questions and suggestions welcome.

 Website:
  http://www.sherlocksoftware.org/
*)

procedure ITD_Cancel;
  external 'itd_cancel@files:itdownload.dll stdcall';

procedure ITD_ClearFiles;
  external 'itd_clearfiles@files:itdownload.dll stdcall';

function ITD_DownloadFile(url: PAnsiChar; destfilename: PAnsiChar): integer;
  external 'itd_downloadfile@files:itdownload.dll stdcall';

function ITD_GetResultLen: integer;
  external 'itd_getresultlen@files:itdownload.dll stdcall';

procedure ITD_GetResultString(buffer: PAnsiChar; maxlen: integer);
  external 'itd_getresultstring@files:itdownload.dll stdcall';

procedure ITD_Internal_InitUI(HostHwnd: dword);
  external 'itd_initui@files:itdownload.dll stdcall';

function ITD_Internal_LoadStrings(filename: PAnsiChar): boolean;
  external 'itd_loadstrings@files:itdownload.dll stdcall';

procedure ITD_Internal_SetOption(option, value: PAnsiChar);
  external 'itd_setoption@files:itdownload.dll stdcall';

function ITD_Internal_GetFileSize(url: PAnsiChar; var size: Cardinal): boolean;
  external 'itd_getfilesize@files:itdownload.dll stdcall';

function ITD_Internal_GetOption(option: PAnsiChar; buffer: PAnsiChar; length: integer): integer;
  external 'itd_getoption@files:itdownload.dll stdcall';

procedure ITD_Internal_SetString(index: integer; value: PAnsiChar);
  external 'itd_setstring@files:itdownload.dll stdcall';

procedure ITD_Internal_AddFile(url: PAnsiChar; destfilename: PAnsiChar);
  external 'itd_addfile@files:itdownload.dll stdcall';

procedure ITD_Internal_AddMirror(url: PAnsiChar; destfilename: PAnsiChar);
  external 'itd_addmirror@files:itdownload.dll stdcall';

procedure ITD_Internal_AddFileSize(url: PAnsiChar; destfilename: PAnsiChar; size: integer);
  external 'itd_addfilesize@files:itdownload.dll stdcall';

function ITD_Internal_DownloadFiles(surface: hwnd): integer;
  external 'itd_downloadfiles@files:itdownload.dll stdcall';

function ITD_FileCount: integer;
  external 'itd_filecount@files:itdownload.dll stdcall';

function ITD_Internal_PostPage(url, buffer: PAnsiChar; length: integer): boolean;
  external 'itd_postpage@files:itdownload.dll stdcall';


const
  ITDERR_SUCCESS = 0;
  ITDERR_USERCANCEL = 1;
  ITDERR_ERROR = 3;

  {Constants for Language String indexes:}
  ITDS_DownloadFailed = 104;

  ITDS_TitleCaption = 200;
  ITDS_TitleDescription = 201;

  ITDS_MessageFailRetryContinue = 250;
  ITDS_MessageFailRetry = 251;

  ITDS_Retry = 502;

  ITD_Event_DownloadPageEntered = 1;
  ITD_Event_DownloadPageLeft = 2;
  ITD_Event_DownloadFailed = 3;

var
  itd_allowcontinue: boolean;
  itd_retryonback: boolean;

  ITD_AfterSuccess: procedure(downloadPage: TWizardPage);
  ITD_EventHandler: procedure(event: integer);

procedure ITD_DownloadFiles();
begin
  ITD_Internal_DownloadFiles(0);
end;

procedure ITD_AddFile(const URL, filename: string);
begin
  ITD_Internal_AddFile(URL, filename);
end;

procedure ITD_AddMirror(const URL, filename: string);
begin
  ITD_Internal_AddMirror(URL, filename);
end;

procedure ITD_AddFileSize(const URL, filename: string; size: integer);
begin
  ITD_Internal_AddFileSize(URL, filename, size);
end;

function ITD_HandleSkipPage(sender: TWizardPage): boolean;
begin
  result := (itd_filecount = 0);
end;

procedure ITD_SetString(index: integer; value: string);
begin
  itd_internal_setstring(index, value);
end;

function ITD_GetFileSize(const url: string; var size: cardinal): boolean;
begin
  result := itd_internal_getfilesize(PAnsiChar(url), size);
end;

function ITD_LoadStrings(const filename: string): boolean;
begin
  result := itd_internal_loadstrings(filename);
end;

function ITD_GetString(index: integer): string;
begin
	case index of
		100: Result := 'Getting file information...';
		101: Result := 'Starting download...';
		102: Result := 'Downloading...';
		103: Result := 'Download complete!';
		104: Result := 'Download failed.';
		105: Result := 'Downloading (%s)...';
		200: Result := 'Downloading additional files';
		201: Result := 'Please wait while setup downloads Microsoft Visual C++ runtime files...';
		250: Result := 'Sorry, the files could not be downloaded. Check your connection and click ''Retry'' to try downloading the files again, or click ''Next'' to continue installing anyway.';
		251: Result := 'Sorry, the files could not be downloaded. Check your connection and click ''Retry'' to try downloading the files again, or click ''Cancel'' to terminate setup.';
		300: Result := 'File:';
		301: Result := 'Speed:';
		302: Result := 'Status:';
		303: Result := 'Elapsed time:';
		304: Result := 'Remaining time:';
		305: Result := 'Current file:';
		306: Result := 'Total progress:';
		400: Result := 'Unknown';
		450: Result := 'second';
		451: Result := 'seconds';
		452: Result := 'minute';
		453: Result := 'minutes';
		454: Result := 'hour';
		455: Result := 'hours';
		456: Result := 'day';
		457: Result := 'days';
		458: Result := '%1 %2/s';
		459: Result := '%1 %2 of %3 %4';
		460: Result := '%1 %2 of unknown';
		461: Result := 'No';
		470: Result := 'B';
		471: Result := 'KB';
		472: Result := 'MB';
		473: Result := 'GB';
		500: Result := 'Details';
		501: Result := 'Hide';
		502: Result := 'Retry';
	end;
end;

procedure ITD_NowDoDownload(sender: TWizardPage);
var err: integer;
begin
  wizardform.backbutton.enabled := false;
  wizardform.nextbutton.enabled := false;

  sender.caption := ITD_GetString(ITDS_TitleCaption);
  sender.description := ITD_GetString(ITDS_TitleDescription);

  err := ITD_Internal_DownloadFiles(sender.surface.handle);

  case err of
    ITDERR_SUCCESS: begin
        wizardform.nextbutton.enabled := true;
        wizardform.nextbutton.onclick(nil);

        if itd_aftersuccess <> nil then
          itd_aftersuccess(sender);
      end;
    ITDERR_USERCANCEL: ; //Don't show a message, this happens on setup close and cancel click
  else begin
    //Some unexpected error, like connection interrupted
      wizardform.backbutton.caption := ITD_GetString(ITDS_Retry);
      wizardform.backbutton.enabled := true;
      wizardform.backbutton.show();
      itd_retryonback := true;

      wizardform.nextbutton.enabled := itd_allowcontinue;

      if ITD_EventHandler <> nil then
        ITD_EventHandler(ITD_Event_DownloadFailed);

      if itd_allowcontinue then begin //Download failed, we can retry, continue, or exit
        sender.caption := ITD_GetString(ITDS_DownloadFailed);
        sender.description := ITD_GetString(ITDS_MessageFailRetryContinue);

        MsgBox(ITD_GetString(ITDS_MessageFailRetryContinue), mbError, MB_OK)
      end else begin //Download failed, we must retry or exit setup
        sender.caption := ITD_GetString(ITDS_DownloadFailed);
        sender.description := ITD_GetString(ITDS_MessageFailRetry);

        MsgBox(ITD_GetString(ITDS_MessageFailRetry), mbError, MB_OK)
      end;
    end;
  end;
end;

procedure ITD_HandleShowPage(sender: TWizardPage);
begin
  wizardform.nextbutton.enabled := false;
  wizardform.backbutton.hide();

  if ITD_EventHandler <> nil then
    ITD_EventHandler(ITD_Event_DownloadPageEntered);

  itd_nowdodownload(sender);
end;

function ITD_HandleBackClick(sender: TWizardpage): boolean;
begin
  result := false;
  if itd_retryonback then begin
    itd_retryonback := false;
    wizardform.backbutton.hide();
    itd_nowdodownload(sender);
  end;
end;

function ITD_HandleNextClick(sender: TWizardpage): boolean;
begin
  if ITD_EventHandler <> nil then
    ITD_EventHandler(ITD_Event_DownloadPageLeft);

  result := true;
end;

procedure ITD_Init;
begin
 //Currently a NOP. Don't count on it in future.
end;

function ITD_PostPage(const url, data: string; out response: string): boolean;
begin
  result := ITD_Internal_PostPage(PAnsiChar(url), PAnsiChar(data), length(data));

  if result then begin
    setlength(response, ITD_GetResultLen);
    ITD_GetResultString(PAnsiChar(response), length(response));
  end;
end;

function ITD_DownloadAfter(afterID: integer): TWizardPage;
var itd_downloadPage: TWizardPage;
begin
  itd_downloadpage := CreateCustomPage(afterID, ITD_GetString(ITDS_TitleCaption), ITD_GetString(ITDS_TitleDescription));

  itd_downloadpage.onactivate := @itd_handleshowpage;
  itd_downloadpage.onshouldskippage := @itd_handleskippage;
  itd_downloadpage.onbackbuttonclick := @itd_handlebackclick;
  itd_downloadpage.onnextbuttonclick := @itd_handlenextclick;

  itd_internal_initui(itd_downloadpage.surface.handle);

  result := itd_downloadpage;
end;

procedure ITD_SetOption(const option, value: string);
begin
  //The options which call ITD_SetString are depreciated, use ITD_SetString directly
  if comparetext(option, 'UI_Caption') = 0 then
    ITD_SetString(ITDS_TitleCaption, value)
  else if comparetext(option, 'UI_Description') = 0 then
    ITD_SetString(ITDS_TitleDescription, value)
  else if comparetext(option, 'UI_FailMessage') = 0 then
    ITD_SetString(ITDS_MessageFailRetry, value)
  else if comparetext(option, 'UI_FailOrContinueMessage') = 0 then
    ITD_SetString(ITDS_MessageFailRetryContinue, value)
  else if comparetext(option, 'UI_AllowContinue') = 0 then
    ITD_AllowContinue := (value = '1')
  else
    ITD_Internal_SetOption(option, value);
end;

function ITD_GetOption(const option: string): string;
begin
  setlength(result, 500);
  setlength(result, itd_internal_getoption(PAnsiChar(option), PAnsiChar(result), length(result)));
end;
