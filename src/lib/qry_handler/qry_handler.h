#ifndef QRY_HANDLER_H
#define QRY_HANDLER_H
#include "../geo_handler/geo_handler.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"

typedef void *Qry; // principais operações do qry_handler

Qry execute_qry_commands(FileData qryFileData, FileData geoFileData,
                         Ground ground, const char *output_path);

void destroy_qry_waste(Qry qry);

#endif // QRY_HANDLER_H