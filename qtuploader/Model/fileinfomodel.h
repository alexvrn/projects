#ifndef FILEINFOMODEL_H
#define FILEINFOMODEL_H

#include "fileshortinfomodel.h"

class FileInfoModel : public FileShortInfoModel
{
public:
    FileInfoModel() { }

    int m_iStatusFile;
};

#endif // FILEINFO_H
