#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <dirent.h>

int listdir(const char *path)
{
  struct dirent *entry;
  DIR *dp;

  dp = opendir(path);
  if (dp == NULL)
  {
      debugPrint("opendir error\n");
    //perror("opendir");
    return -1;
  }

  while((entry = readdir(dp)))
    debugPrint("%s\n", (entry->d_name));

  closedir(dp);
  return 0;
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    listdir("D:\\");

    while(1) {
        Sleep(2000);
    }

    return 0;
}
