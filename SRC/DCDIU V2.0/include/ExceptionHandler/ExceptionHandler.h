/*
============================================================
 File Name    : ExceptionHandler.h
 Module       : Exception Handling Module
 Description  : Declares the ExceptionHandler class responsible
                for centralized error and exception management
                across the DCDIU system.

 Responsibilities:
  - Provide centralized exception handling mechanism
  - Log or display contextual error information
  - Improve system robustness and maintainability
  - Standardize error reporting across modules

============================================================
*/

#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include <string>

using namespace std;

/*
------------------------------------------------------------
 Class Name    : ExceptionHandler
 Description   : Provides static utility functionality for
                handling exceptions and runtime errors
                occurring within different modules of
                the DCDIU system.

                Ensures consistent and structured error
                reporting throughout the application.
------------------------------------------------------------
*/
class ExceptionHandler {

public:

    /*
     * --------------------------------------------------------
     * Function Name : handle()
     * --------------------------------------------------------
     * Description   : Handles exceptions or runtime errors
     *                by reporting contextual information.
     *
     *                This method can be invoked by any
     *                module to process unexpected errors
     *                in a standardized manner.
     *
     * @param context  Descriptive message or context
     *                 indicating where the error occurred.
     *
     * Return          : void
     *
     * Note:
     * This method is static and does not require
     * instantiation of ExceptionHandler.
     * --------------------------------------------------------
     */
    static void handle(const string& context);
};

#endif  // EXCEPTION_HANDLER_H