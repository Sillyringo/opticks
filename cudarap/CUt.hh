#pragma once

struct CUt 
{
    template <typename T>
    static T* UploadArray(const T* array, unsigned num_items ) ;    

    template <typename T>
    static T* DownloadArray(const T* array, unsigned num_items ) ;    
};

