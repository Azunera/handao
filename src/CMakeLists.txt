add_executable(Handao handao.rc
    utils.cpp
    dictionary.cpp
    ResultPopUp.cpp
    main.cpp)
  
target_link_libraries(Handao PRIVATE Tesseract::libtesseract SFML::Graphics SFML::Window)
target_include_directories(Handao PRIVATE ${Tesseract_INCLUDE_DIRS} ${Leptonica_INCLUDE_DIRS})

