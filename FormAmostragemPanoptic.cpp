#include "FormAmostragemPanoptic.h"

#include "FormPrincipal.h"

#include "ThreadParameters.h"
#include "ThreadImage.h"

#include "ogrsf_frmts.h"

#include <stdio.h> 

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

namespace Abilio {

	void FormAmostragemPanoptic::ImportarShapeTrain()
	{

		// onde salvar o roi
		if (dialogImportarShape->ShowDialog() != System::Windows::Forms::DialogResult::OK)
			return;


		// criar arquivo
		String ^filename = dialogImportarShape->FileName;


		char *nom_arquivo = (char*)(void*)Marshal::StringToHGlobalAnsi(filename);


		GDALDriver *poDriver;

		GDALAllRegister();

		GDALDataset *dataset = (GDALDataset*)GDALOpenEx(nom_arquivo, GDAL_OF_VECTOR, NULL, NULL, NULL);

		if (dataset == NULL)
		{
			MessageBox::Show("Erro ao tentar importar!");
			return;
		}

		poDriver = dataset->GetDriver();



		// pegar lat e long do ponto 0
		double xIni, yIni, xFim, yFim, pixelSizeX, pixelSizeY;

		pixelSizeX = image->GetSizePixel();
		pixelSizeY = image->GetSizePixel();


		ImageLib *imageLib = new ImageLib();

		float *vertx, *verty;


		roisTrain = new vector<ROI>();

		int numRoi = 0;

		qtdAmostrasTrain = 0;
		for (int i = 0; i < dataset->GetLayerCount(); i++) {

			OGRLayer *poLayer = dataset->GetLayer(i);


			// georeferencia
			OGRSpatialReference *geoReference = poLayer->GetSpatialRef();
			OGRSpatialReference *geoLatLong = new OGRSpatialReference();
			OGRCoordinateTransformation *coordTrans = NULL;


			if (geoReference->IsProjected()) { // converter para lat long		

				image->GetCoordOrigi(0, 0, &xIni, &yIni);
				image->GetCoordOrigi(image->GetCountRows() - 1, image->GetCountCols() - 1, &xFim, &yFim);

				geoLatLong->SetWellKnownGeogCS("WGS84");
				coordTrans = OGRCreateCoordinateTransformation(geoReference, geoLatLong);
			}
			else {
				image->GetCoordWGS84(0, 0, &xIni, &yIni);
				image->GetCoordWGS84(image->GetCountRows() - 1, image->GetCountCols() - 1, &xFim, &yFim);
			}



			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			poLayer->ResetReading();

			OGRFeature *poFeature;
			int numFeature = 0;
			while ((poFeature = poLayer->GetNextFeature()) != NULL) {

				string string_roi = (char*)(void*)Marshal::StringToHGlobalAnsi((numRoi + 1).ToString());

				ROI roi;

				roi.region = poFeature->GetFieldAsInteger(1);
				roi.countBands = image->GetCountBands();
				roi.countTimes = image->GetCountTimes();
				roi.qtdEspectro = roi.countBands * roi.countTimes;
				roi.number = numFeature + 1;
				roi.time = 1; //form->timeView;
				roi.name = "ROI #" + string_roi;


				OGRGeometry *poGeometry = poFeature->GetGeometryRef();

				if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					roi.shape = 2;

					OGRPoint *point = (OGRPoint *)poGeometry;

					roi.countVertex = 1;


					roi.vertex = (int **)calloc(roi.countVertex, sizeof(int));
					roi.geo = (double **)calloc(roi.countVertex, sizeof(double));


					int p = 0;
					roi.vertex[p] = (int *)calloc(2, sizeof(int));
					roi.geo[p] = (double *)calloc(2, sizeof(int));


					double x = point->getX();
					double y = point->getY();
					double lat = 0;
					double lng = 0;


					// se houver georreferencia
					if (geoReference->IsProjected()) { // converter para lat long
						lat = y;
						lng = x;

						//if (coordTrans) 
						//coordTrans->Transform(1, &lng, &lat);

						x = (lng - xIni) / pixelSizeX;
						y = (yIni - lat) / pixelSizeY;
					}
					else {
						lat = y;
						lng = x;

						//if (coordTrans) 
						//coordTrans->Transform(1, &lng, &lat);

						x = (lng - xIni) * pixelSizeX;
						y = (yIni - lat) * pixelSizeY;
					}


					roi.vertex[p][0] = x;
					roi.vertex[p][1] = y;

					roi.geo[p][0] = lat;
					roi.geo[p][1] = lng;



					int qtdPoints = 1;

					roi.count = qtdPoints;
					roi.countValues = qtdPoints;

					roi.points = (int **)calloc(roi.countValues, sizeof(int));



					roi.points[p] = (int *)calloc(2, sizeof(int));

					roi.points[p][0] = x;
					roi.points[p][1] = y;


					// color
					roi.color = (int *)calloc(3, sizeof(int));

					roi.color[0] = TColors::GetR(roi.region - 1);
					roi.color[1] = TColors::GetG(roi.region - 1);
					roi.color[2] = TColors::GetB(roi.region - 1);

					roisTrain->push_back(roi);


					qtdAmostrasTrain += qtdPoints;

				}
				else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;

					OGRLinearRing *ring = poPolygon->getExteriorRing();

					roi.shape = 1;

					roi.countVertex = ring->getNumPoints();


					roi.vertex = (int **)calloc(roi.countVertex, sizeof(int));
					roi.geo = (double **)calloc(roi.countVertex, sizeof(double));

					for (int p = 0; p < ring->getNumPoints(); p++) {

						roi.vertex[p] = (int *)calloc(2, sizeof(int));
						roi.geo[p] = (double *)calloc(2, sizeof(int));

						OGRPoint point;

						ring->getPoint(p, &point);


						int x = point.getX();
						int y = point.getY();
						double lat = 0;
						double lng = 0;


						// se houver georreferencia
						if (geoReference->IsProjected()) {

							lat = y;
							lng = x;

							//if (coordTrans) 
							//coordTrans->Transform(1, &lng, &lat);


							x = (lng - xIni) / pixelSizeX;
							y = (yIni - lat) / pixelSizeY;
						}


						roi.vertex[p][0] = x;
						roi.vertex[p][1] = y;

						roi.geo[p][0] = lat;
						roi.geo[p][1] = lng;
					}


					vertx = (float *)malloc(sizeof(float *) * roi.countVertex);
					verty = (float *)malloc(sizeof(float *) * roi.countVertex);


					// valores 
					int xmin = 999999;
					int xmax = -999999;
					int ymin = 999999;
					int ymax = -999999;
					for (int j = 0; j < roi.countVertex; j++) { //mod 14/12/2018
						vertx[j] = roi.vertex[j][0]; //mod 14/12/2018
						verty[j] = roi.vertex[j][1]; //mod 14/12/2018


						// min x
						if (vertx[j] < xmin)
							xmin = vertx[j];

						// max x
						if (vertx[j] > xmax)
							xmax = vertx[j];

						// min y
						if (verty[j] < ymin)
							ymin = verty[j];

						// max y
						if (verty[j] > ymax)
							ymax = verty[j];
					}


					int qtdPoints = 0;
					for (int px = xmin; px < xmax; px++) {

						for (int py = ymin; py < ymax; py++) {

							bool pertence = imageLib->isPolygon(roi.countVertex, vertx, verty, px, py); // qtd

							if (pertence)
								qtdPoints++;
						}

					}

					roi.count = qtdPoints;
					roi.countValues = qtdPoints;

					roi.points = (int **)calloc(roi.countValues, sizeof(int));


					int p = 0;
					for (int px = xmin; px < xmax; px++) {

						for (int py = ymin; py < ymax; py++) {

							bool pertence = imageLib->isPolygon(roi.countVertex, vertx, verty, px, py); // qtd

							if (pertence) {
								roi.points[p] = (int *)calloc(2, sizeof(int));

								roi.points[p][0] = px;
								roi.points[p][1] = py;

								p++;
							}
						}
					}


					// color
					roi.color = (int *)calloc(3, sizeof(int));

					roi.color[0] = TColors::GetR(roi.region - 1);
					roi.color[1] = TColors::GetG(roi.region - 1);
					roi.color[2] = TColors::GetB(roi.region - 1);

					roisTrain->push_back(roi);


					qtdAmostrasTrain += qtdPoints;
				}

				OGRFeature::DestroyFeature(poFeature);

				numFeature++;
				numRoi++;
			}

		}

		GDALClose(dataset);

		lblQtdAmostras->Text = "Número de Amostras " + qtdAmostrasTrain.ToString();

	}

	void FormAmostragemPanoptic::ImportarShapeVal()
	{

		// onde salvar o roi
		if (dialogImportarShape->ShowDialog() != System::Windows::Forms::DialogResult::OK)
			return;


		// criar arquivo
		String ^filename = dialogImportarShape->FileName;


		char *nom_arquivo = (char*)(void*)Marshal::StringToHGlobalAnsi(filename);


		GDALDriver *poDriver;

		GDALAllRegister();

		GDALDataset *dataset = (GDALDataset*)GDALOpenEx(nom_arquivo, GDAL_OF_VECTOR, NULL, NULL, NULL);

		if (dataset == NULL)
		{
			MessageBox::Show("Erro ao tentar importar!");
			return;
		}

		poDriver = dataset->GetDriver();



		// pegar lat e long do ponto 0
		double xIni, yIni, xFim, yFim, pixelSizeX, pixelSizeY;

		pixelSizeX = image->GetSizePixel();
		pixelSizeY = image->GetSizePixel();


		ImageLib *imageLib = new ImageLib();

		float *vertx, *verty;


		roisVal = new vector<ROI>();

		int numRoi = 0;

		qtdAmostrasVal = 0;
		for (int i = 0; i < dataset->GetLayerCount(); i++) {

			OGRLayer *poLayer = dataset->GetLayer(i);


			// georeferencia
			OGRSpatialReference *geoReference = poLayer->GetSpatialRef();
			OGRSpatialReference *geoLatLong = new OGRSpatialReference();
			OGRCoordinateTransformation *coordTrans = NULL;


			if (geoReference->IsProjected()) { // converter para lat long		

				image->GetCoordOrigi(0, 0, &xIni, &yIni);
				image->GetCoordOrigi(image->GetCountRows() - 1, image->GetCountCols() - 1, &xFim, &yFim);

				geoLatLong->SetWellKnownGeogCS("WGS84");
				coordTrans = OGRCreateCoordinateTransformation(geoReference, geoLatLong);
			}
			else {
				image->GetCoordWGS84(0, 0, &xIni, &yIni);
				image->GetCoordWGS84(image->GetCountRows() - 1, image->GetCountCols() - 1, &xFim, &yFim);
			}



			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			poLayer->ResetReading();

			OGRFeature *poFeature;
			int numFeature = 0;
			while ((poFeature = poLayer->GetNextFeature()) != NULL) {

				string string_roi = (char*)(void*)Marshal::StringToHGlobalAnsi((numRoi + 1).ToString());

				ROI roi;

				roi.region = poFeature->GetFieldAsInteger(1);
				roi.countBands = image->GetCountBands();
				roi.countTimes = image->GetCountTimes();
				roi.qtdEspectro = roi.countBands * roi.countTimes;
				roi.number = numFeature + 1;
				roi.time = 1; //form->timeView;
				roi.name = "ROI #" + string_roi;


				OGRGeometry *poGeometry = poFeature->GetGeometryRef();

				if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					roi.shape = 2;

					OGRPoint *point = (OGRPoint *)poGeometry;

					roi.countVertex = 1;


					roi.vertex = (int **)calloc(roi.countVertex, sizeof(int));
					roi.geo = (double **)calloc(roi.countVertex, sizeof(double));


					int p = 0;
					roi.vertex[p] = (int *)calloc(2, sizeof(int));
					roi.geo[p] = (double *)calloc(2, sizeof(int));


					double x = point->getX();
					double y = point->getY();
					double lat = 0;
					double lng = 0;


					// se houver georreferencia
					if (geoReference->IsProjected()) { // converter para lat long
						lat = y;
						lng = x;

						//if (coordTrans) 
						//coordTrans->Transform(1, &lng, &lat);

						x = (lng - xIni) / pixelSizeX;
						y = (yIni - lat) / pixelSizeY;
					}
					else {
						lat = y;
						lng = x;

						//if (coordTrans) 
						//coordTrans->Transform(1, &lng, &lat);

						x = (lng - xIni) * pixelSizeX;
						y = (yIni - lat) * pixelSizeY;
					}


					roi.vertex[p][0] = x;
					roi.vertex[p][1] = y;

					roi.geo[p][0] = lat;
					roi.geo[p][1] = lng;



					int qtdPoints = 1;

					roi.count = qtdPoints;
					roi.countValues = qtdPoints;

					roi.points = (int **)calloc(roi.countValues, sizeof(int));



					roi.points[p] = (int *)calloc(2, sizeof(int));

					roi.points[p][0] = x;
					roi.points[p][1] = y;


					// color
					roi.color = (int *)calloc(3, sizeof(int));

					roi.color[0] = TColors::GetR(roi.region - 1);
					roi.color[1] = TColors::GetG(roi.region - 1);
					roi.color[2] = TColors::GetB(roi.region - 1);

					roisVal->push_back(roi);


					qtdAmostrasVal += qtdPoints;

				}
				else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;

					OGRLinearRing *ring = poPolygon->getExteriorRing();

					roi.shape = 1;

					roi.countVertex = ring->getNumPoints();


					roi.vertex = (int **)calloc(roi.countVertex, sizeof(int));
					roi.geo = (double **)calloc(roi.countVertex, sizeof(double));

					for (int p = 0; p < ring->getNumPoints(); p++) {

						roi.vertex[p] = (int *)calloc(2, sizeof(int));
						roi.geo[p] = (double *)calloc(2, sizeof(int));

						OGRPoint point;

						ring->getPoint(p, &point);


						int x = point.getX();
						int y = point.getY();
						double lat = 0;
						double lng = 0;


						// se houver georreferencia
						if (geoReference->IsProjected()) {

							lat = y;
							lng = x;

							//if (coordTrans) 
							//coordTrans->Transform(1, &lng, &lat);


							x = (lng - xIni) / pixelSizeX;
							y = (yIni - lat) / pixelSizeY;
						}


						roi.vertex[p][0] = x;
						roi.vertex[p][1] = y;

						roi.geo[p][0] = lat;
						roi.geo[p][1] = lng;
					}


					vertx = (float *)malloc(sizeof(float *) * roi.countVertex);
					verty = (float *)malloc(sizeof(float *) * roi.countVertex);


					// valores 
					int xmin = 999999;
					int xmax = -999999;
					int ymin = 999999;
					int ymax = -999999;
					for (int j = 0; j < roi.countVertex; j++) { //mod 14/12/2018
						vertx[j] = roi.vertex[j][0]; //mod 14/12/2018
						verty[j] = roi.vertex[j][1]; //mod 14/12/2018


						// min x
						if (vertx[j] < xmin)
							xmin = vertx[j];

						// max x
						if (vertx[j] > xmax)
							xmax = vertx[j];

						// min y
						if (verty[j] < ymin)
							ymin = verty[j];

						// max y
						if (verty[j] > ymax)
							ymax = verty[j];
					}


					int qtdPoints = 0;
					for (int px = xmin; px < xmax; px++) {

						for (int py = ymin; py < ymax; py++) {

							bool pertence = imageLib->isPolygon(roi.countVertex, vertx, verty, px, py); // qtd

							if (pertence)
								qtdPoints++;
						}

					}

					roi.count = qtdPoints;
					roi.countValues = qtdPoints;

					roi.points = (int **)calloc(roi.countValues, sizeof(int));


					int p = 0;
					for (int px = xmin; px < xmax; px++) {

						for (int py = ymin; py < ymax; py++) {

							bool pertence = imageLib->isPolygon(roi.countVertex, vertx, verty, px, py); // qtd

							if (pertence) {
								roi.points[p] = (int *)calloc(2, sizeof(int));

								roi.points[p][0] = px;
								roi.points[p][1] = py;

								p++;
							}
						}
					}


					// color
					roi.color = (int *)calloc(3, sizeof(int));

					roi.color[0] = TColors::GetR(roi.region - 1);
					roi.color[1] = TColors::GetG(roi.region - 1);
					roi.color[2] = TColors::GetB(roi.region - 1);

					roisVal->push_back(roi);


					qtdAmostrasVal += qtdPoints;
				}

				OGRFeature::DestroyFeature(poFeature);

				numFeature++;
				numRoi++;
			}

		}

		GDALClose(dataset);

		label4->Text = "Número de Amostras " + qtdAmostrasVal.ToString();

	}

	void FormAmostragemPanoptic::ImportarShapeTest()
	{

		// onde salvar o roi
		if (dialogImportarShape->ShowDialog() != System::Windows::Forms::DialogResult::OK)
			return;


		// criar arquivo
		String ^filename = dialogImportarShape->FileName;


		char *nom_arquivo = (char*)(void*)Marshal::StringToHGlobalAnsi(filename);


		GDALDriver *poDriver;

		GDALAllRegister();

		GDALDataset *dataset = (GDALDataset*)GDALOpenEx(nom_arquivo, GDAL_OF_VECTOR, NULL, NULL, NULL);

		if (dataset == NULL)
		{
			MessageBox::Show("Erro ao tentar importar!");
			return;
		}

		poDriver = dataset->GetDriver();



		// pegar lat e long do ponto 0
		double xIni, yIni, xFim, yFim, pixelSizeX, pixelSizeY;

		pixelSizeX = image->GetSizePixel();
		pixelSizeY = image->GetSizePixel();


		ImageLib *imageLib = new ImageLib();

		float *vertx, *verty;


		roisTest = new vector<ROI>();

		int numRoi = 0;

		qtdAmostrasTest = 0;
		for (int i = 0; i < dataset->GetLayerCount(); i++) {

			OGRLayer *poLayer = dataset->GetLayer(i);


			// georeferencia
			OGRSpatialReference *geoReference = poLayer->GetSpatialRef();
			OGRSpatialReference *geoLatLong = new OGRSpatialReference();
			OGRCoordinateTransformation *coordTrans = NULL;


			if (geoReference->IsProjected()) { // converter para lat long		

				image->GetCoordOrigi(0, 0, &xIni, &yIni);
				image->GetCoordOrigi(image->GetCountRows() - 1, image->GetCountCols() - 1, &xFim, &yFim);

				geoLatLong->SetWellKnownGeogCS("WGS84");
				coordTrans = OGRCreateCoordinateTransformation(geoReference, geoLatLong);
			}
			else {
				image->GetCoordWGS84(0, 0, &xIni, &yIni);
				image->GetCoordWGS84(image->GetCountRows() - 1, image->GetCountCols() - 1, &xFim, &yFim);
			}



			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			poLayer->ResetReading();

			OGRFeature *poFeature;
			int numFeature = 0;
			while ((poFeature = poLayer->GetNextFeature()) != NULL) {

				string string_roi = (char*)(void*)Marshal::StringToHGlobalAnsi((numRoi + 1).ToString());

				ROI roi;

				roi.region = poFeature->GetFieldAsInteger(1);
				roi.countBands = image->GetCountBands();
				roi.countTimes = image->GetCountTimes();
				roi.qtdEspectro = roi.countBands * roi.countTimes;
				roi.number = numFeature + 1;
				roi.time = 1; //form->timeView;
				roi.name = "ROI #" + string_roi;


				OGRGeometry *poGeometry = poFeature->GetGeometryRef();

				if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					roi.shape = 2;

					OGRPoint *point = (OGRPoint *)poGeometry;

					roi.countVertex = 1;


					roi.vertex = (int **)calloc(roi.countVertex, sizeof(int));
					roi.geo = (double **)calloc(roi.countVertex, sizeof(double));


					int p = 0;
					roi.vertex[p] = (int *)calloc(2, sizeof(int));
					roi.geo[p] = (double *)calloc(2, sizeof(int));


					double x = point->getX();
					double y = point->getY();
					double lat = 0;
					double lng = 0;


					// se houver georreferencia
					if (geoReference->IsProjected()) { // converter para lat long
						lat = y;
						lng = x;

						//if (coordTrans) 
						//coordTrans->Transform(1, &lng, &lat);

						x = (lng - xIni) / pixelSizeX;
						y = (yIni - lat) / pixelSizeY;
					}
					else {
						lat = y;
						lng = x;

						//if (coordTrans) 
						//coordTrans->Transform(1, &lng, &lat);

						x = (lng - xIni) * pixelSizeX;
						y = (yIni - lat) * pixelSizeY;
					}


					roi.vertex[p][0] = x;
					roi.vertex[p][1] = y;

					roi.geo[p][0] = lat;
					roi.geo[p][1] = lng;



					int qtdPoints = 1;

					roi.count = qtdPoints;
					roi.countValues = qtdPoints;

					roi.points = (int **)calloc(roi.countValues, sizeof(int));



					roi.points[p] = (int *)calloc(2, sizeof(int));

					roi.points[p][0] = x;
					roi.points[p][1] = y;


					// color
					roi.color = (int *)calloc(3, sizeof(int));

					roi.color[0] = TColors::GetR(roi.region - 1);
					roi.color[1] = TColors::GetG(roi.region - 1);
					roi.color[2] = TColors::GetB(roi.region - 1);

					roisTest->push_back(roi);


					qtdAmostrasTest += qtdPoints;

				}
				else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;

					OGRLinearRing *ring = poPolygon->getExteriorRing();

					roi.shape = 1;

					roi.countVertex = ring->getNumPoints();


					roi.vertex = (int **)calloc(roi.countVertex, sizeof(int));
					roi.geo = (double **)calloc(roi.countVertex, sizeof(double));

					for (int p = 0; p < ring->getNumPoints(); p++) {

						roi.vertex[p] = (int *)calloc(2, sizeof(int));
						roi.geo[p] = (double *)calloc(2, sizeof(int));

						OGRPoint point;

						ring->getPoint(p, &point);


						int x = point.getX();
						int y = point.getY();
						double lat = 0;
						double lng = 0;


						// se houver georreferencia
						if (geoReference->IsProjected()) {

							lat = y;
							lng = x;

							//if (coordTrans) 
							//coordTrans->Transform(1, &lng, &lat);


							x = (lng - xIni) / pixelSizeX;
							y = (yIni - lat) / pixelSizeY;
						}


						roi.vertex[p][0] = x;
						roi.vertex[p][1] = y;

						roi.geo[p][0] = lat;
						roi.geo[p][1] = lng;
					}


					vertx = (float *)malloc(sizeof(float *) * roi.countVertex);
					verty = (float *)malloc(sizeof(float *) * roi.countVertex);


					// valores 
					int xmin = 999999;
					int xmax = -999999;
					int ymin = 999999;
					int ymax = -999999;
					for (int j = 0; j < roi.countVertex; j++) { //mod 14/12/2018
						vertx[j] = roi.vertex[j][0]; //mod 14/12/2018
						verty[j] = roi.vertex[j][1]; //mod 14/12/2018


						// min x
						if (vertx[j] < xmin)
							xmin = vertx[j];

						// max x
						if (vertx[j] > xmax)
							xmax = vertx[j];

						// min y
						if (verty[j] < ymin)
							ymin = verty[j];

						// max y
						if (verty[j] > ymax)
							ymax = verty[j];
					}


					int qtdPoints = 0;
					for (int px = xmin; px < xmax; px++) {

						for (int py = ymin; py < ymax; py++) {

							bool pertence = imageLib->isPolygon(roi.countVertex, vertx, verty, px, py); // qtd

							if (pertence)
								qtdPoints++;
						}

					}

					roi.count = qtdPoints;
					roi.countValues = qtdPoints;

					roi.points = (int **)calloc(roi.countValues, sizeof(int));


					int p = 0;
					for (int px = xmin; px < xmax; px++) {

						for (int py = ymin; py < ymax; py++) {

							bool pertence = imageLib->isPolygon(roi.countVertex, vertx, verty, px, py); // qtd

							if (pertence) {
								roi.points[p] = (int *)calloc(2, sizeof(int));

								roi.points[p][0] = px;
								roi.points[p][1] = py;

								p++;
							}
						}
					}


					// color
					roi.color = (int *)calloc(3, sizeof(int));

					roi.color[0] = TColors::GetR(roi.region - 1);
					roi.color[1] = TColors::GetG(roi.region - 1);
					roi.color[2] = TColors::GetB(roi.region - 1);

					roisTest->push_back(roi);


					qtdAmostrasTest += qtdPoints;
				}

				OGRFeature::DestroyFeature(poFeature);

				numFeature++;
				numRoi++;
			}

		}

		GDALClose(dataset);

		label7->Text = "Número de Amostras " + qtdAmostrasTest.ToString();

	}

	void FormAmostragemPanoptic::Aplicar()
	{
		TImagem *imgAmostra, *imgLabel, *imgPng, *imgClassificada;
		TClasses *classes;
		int row, col, time, b, indRow, indCol, region, ind, indRoi, pjlin, pjcol, r, c, indPonto, indR, size, aux, aux2;
		int linhaAdd, pmetadeRow, p1;
		int colunaAdd, pmetadeCol, p2, indImg;
		float pval, pvalMascara, pvalor;
		int *bandasCorte, numBandasCorte, pvalClass;
		double lat, lon;
		String ^fileAmostra;
		String ^fileLabel;
		String ^filePng;
		String ^fileClassificada;
		String ^fileClassificadaPng;
		char *nom_arquivo_amostra;
		char *nom_arquivo_label;
		char *nom_arquivo_png;
		char *nom_arquivo_classificada;
		char *nom_arquivo_classificada_png;
		bool valido;
		float valR, valG, valB;
		List<float> pivosDiferentes, pivosR, pivosG, pivosB, pivosClass;
		List<int> pivosAreas, pivosXMin, pivosXMax, pivosYMin, pivosYMax;
		cv::string category;
		int area, xMin, xMax, yMin, yMax, x, y, pvalClass2;
		int boxX, boxY, boxWidth, boxHeight, indContador;
		List<int> categoriesIgnore;



		if (image == NULL) {
			MessageBox::Show("Carregue a imagem!");
			return;
		}

		if (imageClassificada == NULL) {
			MessageBox::Show("Carregue a imagem classificada!");
			return;
		}

		if (imageSequencial == NULL) {
			MessageBox::Show("Carregue a imagem sequencial!");
			return;
		}

		if (qtdAmostrasTrain == 0) {
			MessageBox::Show("Carregue o shape train!");
			return;
		}

		if (image->GetCountRows() != imageClassificada->GetCountRows() || image->GetCountCols() != imageClassificada->GetCountCols()) {
			MessageBox::Show("As imagens não possuem a mesma dimensão espacial!");
			return;
		}

		if (imageSequencial->GetCountRows() != imageClassificada->GetCountRows() || imageSequencial->GetCountCols() != imageClassificada->GetCountCols()) {
			MessageBox::Show("As imagens não possuem a mesma dimensão espacial!");
			return;
		}

		if (listBoxBandas->SelectedItems->Count == 0) {
			MessageBox::Show("Selecione no mínimo 1 banda!");
			return;
		}



		// pegar parametros
		linhaAdd = (int)numJanLinhas->Value;
		colunaAdd = (int)numJanColunas->Value;



		// salvar como
		// onde salvar o roi
		if (folderBrowserDialog1->ShowDialog() != System::Windows::Forms::DialogResult::OK)
			return;


		// criar arquivo
		String ^path = folderBrowserDialog1->SelectedPath;



		// montar classes
		classes = imageClassificada->GetClasses();


		pmetadeRow = (linhaAdd - 1) / 2;
		pmetadeCol = (colunaAdd - 1) / 2;


		int qtdRoisLoading = roisTrain->size();

		if (qtdAmostrasVal > 0)
			qtdRoisLoading += roisVal->size();

		if (qtdAmostrasTest > 0)
			qtdRoisLoading += roisTest->size();

		safe_cast<FormPrincipal ^>(this->formPrincipal)->Loading(qtdRoisLoading);



		// travar form
		this->Enabled = false;



		// criar pastas
		String ^pathAmostrasTrain = path + "\\" + "image_train";
		String ^pathLabelsTrain = path + "\\" + "panoptic_train";
		String ^pathClassesTrain = path + "\\" + "class_train";
		String ^pathAnnotations = path + "\\" + "annotations";


		std::string nom_path_amostras_train = (char*)(void*)Marshal::StringToHGlobalAnsi(pathAmostrasTrain);
		std::string nom_path_labes_train = (char*)(void*)Marshal::StringToHGlobalAnsi(pathLabelsTrain);
		std::string nom_path_classes_train = (char*)(void*)Marshal::StringToHGlobalAnsi(pathClassesTrain);
		std::string nom_path_annotations = (char*)(void*)Marshal::StringToHGlobalAnsi(pathAnnotations);

		mkdir(nom_path_amostras_train.c_str());
		mkdir(nom_path_labes_train.c_str());
		mkdir(nom_path_classes_train.c_str());
		mkdir(nom_path_annotations.c_str());



		String ^pathAmostrasVal = path + "\\" + "image_val";
		String ^pathLabelsVal = path + "\\" + "panoptic_val";
		String ^pathClassesVal = path + "\\" + "class_val";

		if (qtdAmostrasVal > 0) {
			std::string nom_path_amostras_val = (char*)(void*)Marshal::StringToHGlobalAnsi(pathAmostrasVal);
			std::string nom_path_labes_val = (char*)(void*)Marshal::StringToHGlobalAnsi(pathLabelsVal);
			std::string nom_path_classes_val = (char*)(void*)Marshal::StringToHGlobalAnsi(pathClassesVal);

			mkdir(nom_path_amostras_val.c_str());
			mkdir(nom_path_labes_val.c_str());
			mkdir(nom_path_classes_val.c_str());
		}



		String ^pathAmostrasTest = path + "\\" + "image_test";
		String ^pathLabelsTest = path + "\\" + "panoptic_test";
		String ^pathClassesTest = path + "\\" + "class_test";

		if (qtdAmostrasTest > 0) {
			std::string nom_path_amostras_test = (char*)(void*)Marshal::StringToHGlobalAnsi(pathAmostrasTest);
			std::string nom_path_labes_test = (char*)(void*)Marshal::StringToHGlobalAnsi(pathLabelsTest);
			std::string nom_path_classes_test = (char*)(void*)Marshal::StringToHGlobalAnsi(pathClassesTest);

			mkdir(nom_path_amostras_test.c_str());
			mkdir(nom_path_labes_test.c_str());
			mkdir(nom_path_classes_test.c_str());
		}



		// bandas selecionadas
		numBandasCorte = listBoxBandas->SelectedItems->Count;

		bandasCorte = TVector::AllocInt(numBandasCorte);

		for (int i = 0; i < listBoxBandas->SelectedItems->Count; i++)
			bandasCorte[i] = listBoxBandas->SelectedIndices[i];



		// recortar amostras
		GDALDataset *datasetIn, *datasetOut;
		GDALDataset *datasetInClass, *datasetOutClass;
		GDALDataset *datasetInSeq, *datasetOutSeq;
		GDALRasterBand *databandIn, *databandOut, *databandInClass, *databandOutClass, *databandInSeq, *databandOutSeq;
		float *recorte, *recorteR, *recorteG, *recorteB, *recorteClass;

		size = linhaAdd * linhaAdd;


		recorte = TVector::AllocFloat(size);
		recorteClass = TVector::AllocFloat(size);
		recorteR = TVector::AllocFloat(size);
		recorteG = TVector::AllocFloat(size);
		recorteB = TVector::AllocFloat(size);


		datasetIn = image->GetDataSet();
		datasetInClass = imageClassificada->GetDataSet();
		datasetInSeq = imageSequencial->GetDataSet();



		// annotations train
		StreamWriter ^jsonPanopticTrain = gcnew StreamWriter(pathAnnotations + "\\" + "panoptic_train.json");
		StreamWriter ^jsonInstanceTrain = gcnew StreamWriter(pathAnnotations + "\\" + "instance_train.json");



		// inicio do arquivo panoptic
		jsonPanopticTrain->WriteLine("{");

		jsonPanopticTrain->WriteLine("\"info\": {\"description\": \"BSB Aerial Dataset\", \"url\": \"None\", \"version\": \"1\", \"year\": 2021, \"contributor\": \"Osmar Carvalho\", \"date_created\": \"07 / 01 / 2021\"},");
		jsonPanopticTrain->WriteLine("\"licenses\": [{\"url\": \"None\", \"id\": 0, \"name\": \"Test License\"}],");

		jsonPanopticTrain->WriteLine("\"images\": [");


		// inicio do arquivo instance
		jsonInstanceTrain->WriteLine("{");

		jsonInstanceTrain->WriteLine("\"info\": {\"description\": \"BSB Aerial Dataset\", \"url\": \"None\", \"version\": \"1\", \"year\": 2021, \"contributor\": \"Osmar Carvalho\", \"date_created\": \"07 / 01 / 2021\"},");
		jsonInstanceTrain->WriteLine("\"licenses\": [{\"url\": \"None\", \"id\": 0, \"name\": \"Test License\"}],");

		jsonInstanceTrain->WriteLine("\"images\": [");


		ind = 1; //variar comeca em 1
		for (r = 0; r < roisTrain->size(); r++) {

			ROI proi = roisTrain->at(r);

			for (c = 0; c < proi.countValues; c++) {

				col = proi.points[c][0];
				row = proi.points[c][1];

				p1 = row - pmetadeRow;
				p2 = col - pmetadeCol;


				// validar tamanho da imagem
				valido = true;
				if (p2 < 0 || (p2 + linhaAdd) > image->GetCountCols() - 1)
					valido = false;
				else if (p1 < 0 || (p1 + linhaAdd) > image->GetCountRows() - 1)
					valido = false;



				if (valido) {

					// nome imagem
					fileAmostra = ind.ToString() + ".tiff";

					jsonPanopticTrain->WriteLine("{\"license\": 0, \"file_name\" : \"" + fileAmostra + "\", \"width\" : " + linhaAdd.ToString() + ", \"height\" : " + linhaAdd.ToString() + ", \"id\" : " + ind.ToString() + "},");

					jsonInstanceTrain->WriteLine("{\"license\": 0, \"file_name\" : \"" + fileAmostra + "\", \"width\" : " + linhaAdd.ToString() + ", \"height\" : " + linhaAdd.ToString() + ", \"id\" : " + ind.ToString() + "},");

					ind++;
				}

			}

		}

		jsonPanopticTrain->WriteLine("],");
		jsonInstanceTrain->WriteLine("],");


		jsonPanopticTrain->WriteLine("\"annotations\": [");
		jsonInstanceTrain->WriteLine("\"annotations\": [");





		// annotations val
		StreamWriter ^jsonPanopticVal;
		StreamWriter ^jsonInstanceVal;

		if (qtdAmostrasVal > 0) {

			jsonPanopticVal = gcnew StreamWriter(pathAnnotations + "\\" + "panoptic_val.json");
			jsonInstanceVal = gcnew StreamWriter(pathAnnotations + "\\" + "instance_val.json");



			// inicio do arquivo panoptic
			jsonPanopticVal->WriteLine("{");

			jsonPanopticVal->WriteLine("\"info\": {\"description\": \"BSB Aerial Dataset\", \"url\": \"None\", \"version\": \"1\", \"year\": 2021, \"contributor\": \"Osmar Carvalho\", \"date_created\": \"07 / 01 / 2021\"},");
			jsonPanopticVal->WriteLine("\"licenses\": [{\"url\": \"None\", \"id\": 0, \"name\": \"Test License\"}],");

			jsonPanopticVal->WriteLine("\"images\": [");


			// inicio do arquivo instance
			jsonInstanceVal->WriteLine("{");

			jsonInstanceVal->WriteLine("\"info\": {\"description\": \"BSB Aerial Dataset\", \"url\": \"None\", \"version\": \"1\", \"year\": 2021, \"contributor\": \"Osmar Carvalho\", \"date_created\": \"07 / 01 / 2021\"},");
			jsonInstanceVal->WriteLine("\"licenses\": [{\"url\": \"None\", \"id\": 0, \"name\": \"Test License\"}],");

			jsonInstanceVal->WriteLine("\"images\": [");


			ind = 1; //variar comeca em 1
			for (r = 0; r < roisVal->size(); r++) {

				ROI proi = roisVal->at(r);

				for (c = 0; c < proi.countValues; c++) {

					col = proi.points[c][0];
					row = proi.points[c][1];

					p1 = row - pmetadeRow;
					p2 = col - pmetadeCol;


					// validar tamanho da imagem
					valido = true;
					if (p2 < 0 || (p2 + linhaAdd) > image->GetCountCols() - 1)
						valido = false;
					else if (p1 < 0 || (p1 + linhaAdd) > image->GetCountRows() - 1)
						valido = false;



					if (valido) {

						// nome imagem
						fileAmostra = ind.ToString() + ".tiff";

						jsonPanopticVal->WriteLine("{\"license\": 0, \"file_name\" : \"" + fileAmostra + "\", \"width\" : " + linhaAdd.ToString() + ", \"height\" : " + linhaAdd.ToString() + ", \"id\" : " + ind.ToString() + "},");

						jsonInstanceVal->WriteLine("{\"license\": 0, \"file_name\" : \"" + fileAmostra + "\", \"width\" : " + linhaAdd.ToString() + ", \"height\" : " + linhaAdd.ToString() + ", \"id\" : " + ind.ToString() + "},");

						ind++;
					}

				}

			}

			jsonPanopticVal->WriteLine("],");
			jsonInstanceVal->WriteLine("],");


			jsonPanopticVal->WriteLine("\"annotations\": [");
			jsonInstanceVal->WriteLine("\"annotations\": [");

		}




		// annotations test
		StreamWriter ^jsonPanopticTest;
		StreamWriter ^jsonInstanceTest;

		if (qtdAmostrasTest > 0) {

			jsonPanopticTest = gcnew StreamWriter(pathAnnotations + "\\" + "panoptic_test.json");
			jsonInstanceTest = gcnew StreamWriter(pathAnnotations + "\\" + "instance_test.json");



			// inicio do arquivo panoptic
			jsonPanopticTest->WriteLine("{");

			jsonPanopticTest->WriteLine("\"info\": {\"description\": \"BSB Aerial Dataset\", \"url\": \"None\", \"version\": \"1\", \"year\": 2021, \"contributor\": \"Osmar Carvalho\", \"date_created\": \"07 / 01 / 2021\"},");
			jsonPanopticTest->WriteLine("\"licenses\": [{\"url\": \"None\", \"id\": 0, \"name\": \"Test License\"}],");

			jsonPanopticTest->WriteLine("\"images\": [");


			// inicio do arquivo instance
			jsonInstanceTest->WriteLine("{");

			jsonInstanceTest->WriteLine("\"info\": {\"description\": \"BSB Aerial Dataset\", \"url\": \"None\", \"version\": \"1\", \"year\": 2021, \"contributor\": \"Osmar Carvalho\", \"date_created\": \"07 / 01 / 2021\"},");
			jsonInstanceTest->WriteLine("\"licenses\": [{\"url\": \"None\", \"id\": 0, \"name\": \"Test License\"}],");

			jsonInstanceTest->WriteLine("\"images\": [");


			ind = 1; //variar comeca em 1
			for (r = 0; r < roisTest->size(); r++) {

				ROI proi = roisTest->at(r);

				for (c = 0; c < proi.countValues; c++) {

					col = proi.points[c][0];
					row = proi.points[c][1];

					p1 = row - pmetadeRow;
					p2 = col - pmetadeCol;


					// validar tamanho da imagem
					valido = true;
					if (p2 < 0 || (p2 + linhaAdd) > image->GetCountCols() - 1)
						valido = false;
					else if (p1 < 0 || (p1 + linhaAdd) > image->GetCountRows() - 1)
						valido = false;



					if (valido) {

						// nome imagem
						fileAmostra = ind.ToString() + ".tiff";

						jsonPanopticTest->WriteLine("{\"license\": 0, \"file_name\" : \"" + fileAmostra + "\", \"width\" : " + linhaAdd.ToString() + ", \"height\" : " + linhaAdd.ToString() + ", \"id\" : " + ind.ToString() + "},");

						jsonInstanceTest->WriteLine("{\"license\": 0, \"file_name\" : \"" + fileAmostra + "\", \"width\" : " + linhaAdd.ToString() + ", \"height\" : " + linhaAdd.ToString() + ", \"id\" : " + ind.ToString() + "},");

						ind++;
					}

				}

			}

			jsonPanopticTest->WriteLine("],");
			jsonInstanceTest->WriteLine("],");


			jsonPanopticTest->WriteLine("\"annotations\": [");
			jsonInstanceTest->WriteLine("\"annotations\": [");

		}
		

		categoriesIgnore.Clear();
		categoriesIgnore.Add(0);
		categoriesIgnore.Add(1);
		categoriesIgnore.Add(2);
		categoriesIgnore.Add(3);


		ind = 1; //variar comeca em 1
		indContador = 1;
		for (r = 0; r < roisTrain->size(); r++) {

			ROI proi = roisTrain->at(r);

			for (c = 0; c < proi.countValues; c++) {

				col = proi.points[c][0];
				row = proi.points[c][1];

				p1 = row - pmetadeRow;
				p2 = col - pmetadeCol;


				// validar tamanho da imagem
				valido = true;
				if (p2 < 0 || (p2 + linhaAdd) > image->GetCountCols() - 1)
					valido = false;
				else if (p1 < 0 || (p1 + linhaAdd) > image->GetCountRows() - 1)
					valido = false;



				if (valido) {

					// nome imagem
					fileAmostra = pathAmostrasTrain + "\\" + ind.ToString() + ".tiff";
					nom_arquivo_amostra = (char*)(void*)Marshal::StringToHGlobalAnsi(fileAmostra);


					fileLabel = pathLabelsTrain + "\\" + "img" + "_" + ind.ToString() + ".mem";
					nom_arquivo_label = (char*)(void*)Marshal::StringToHGlobalAnsi(fileLabel);


					filePng = pathLabelsTrain + "\\" + ind.ToString() + ".png";
					nom_arquivo_png = (char*)(void*)Marshal::StringToHGlobalAnsi(filePng);


					fileClassificada = pathClassesTrain + "\\" + ind.ToString() + ".mem";
					nom_arquivo_classificada = (char*)(void*)Marshal::StringToHGlobalAnsi(fileClassificada);


					fileClassificadaPng = pathClassesTrain + "\\" + ind.ToString() + ".png";
					nom_arquivo_classificada_png = (char*)(void*)Marshal::StringToHGlobalAnsi(fileClassificadaPng);




					// criar imagem
					imgAmostra = new TImagem();

					imgAmostra->isBufferRead = false;
					imgAmostra->isBufferWrite = false;

					imgAmostra->Create(nom_arquivo_amostra, TImagem::GTIFF, 1, numBandasCorte, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);


					imgLabel = new TImagem();

					imgLabel->isBufferRead = false;
					imgLabel->isBufferWrite = false;

					imgLabel->Create(nom_arquivo_label, TImagem::MEMORY, 1, 3, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);


					imgClassificada = new TImagem();

					imgClassificada->isBufferRead = false;
					imgClassificada->isBufferWrite = false;

					imgClassificada->Create(nom_arquivo_classificada, TImagem::MEMORY, 1, 1, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);




					// pegar recorte da imagem
					datasetOut = imgAmostra->GetDataSet();

					for (b = 0; b < numBandasCorte; b++) {
						databandIn = datasetIn->GetRasterBand(bandasCorte[b] + 1);
						databandIn->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOut = datasetOut->GetRasterBand(b + 1);
						databandOut->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);
					}

					imgAmostra->Close();



					// pegar recorte da classificada
					datasetOutClass = imgClassificada->GetDataSet();

					databandInClass = datasetInClass->GetRasterBand(1);
					databandInClass->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorteClass, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

					databandOutClass = datasetOutClass->GetRasterBand(1);
					databandOutClass->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteClass, linhaAdd, linhaAdd, GDT_Float32, 0, 0);


					// pegar recorte da sequencial
					datasetOutSeq = imgLabel->GetDataSet();

					databandInSeq = datasetInSeq->GetRasterBand(1);
					databandInSeq->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);



					// calcular R, G, B para cada ponto
					pivosDiferentes.Clear();
					pivosR.Clear();
					pivosG.Clear();
					pivosB.Clear();
					pivosClass.Clear();

					// limpar recortes
					for (indR = 0; indR < size; indR++) {
						recorteR[indR] = 0;
						recorteG[indR] = 0;
						recorteB[indR] = 0;
					}

					for (indR = 0; indR < size; indR++) {

						pval = recorte[indR];
						pvalClass = (int)recorteClass[indR];

						if (pvalClass > 0) {

							// conversor base 256
							valR = fmod(pval, 256);
							aux = floor(pval / 256);
							valG = fmod(aux, 256);
							valB = floor(aux / 256);

							recorteR[indR] = valR;
							recorteG[indR] = valG;
							recorteB[indR] = valB;


							bool existe = false;
							if (pval > 0) {

								for (int j = 0; j < pivosDiferentes.Count; j++) {

									if (pval == pivosDiferentes[j]) {
										existe = true;
										break;
									}

								}


								if (!existe) {
									pivosDiferentes.Add(pval);
									pivosR.Add(valR);
									pivosG.Add(valG);
									pivosB.Add(valB);
									pivosClass.Add(pvalClass);
								}

							}

						}

					}



					// qtd de pivos
					jsonPanopticTrain->WriteLine("{");
					jsonPanopticTrain->WriteLine("\"segments_info\": ");
					jsonPanopticTrain->WriteLine("[");

					//String texto = "";
					for (indR = 0; indR < pivosDiferentes.Count; indR++) {

						valR = pivosR[indR];
						valG = pivosG[indR];
						valB = pivosB[indR];
						pvalClass = pivosClass[indR];
						pval = pivosDiferentes[indR];

						area = 0;
						xMin = 999999;
						xMax = -999999;
						yMin = 999999;
						yMax = -999999;

						x = 0;
						y = 0;


						cv::Mat img = cv::Mat::zeros(linhaAdd, linhaAdd, CV_8UC1);

						for (int indS = 0; indS < size; indS++) {

							pvalClass2 = (int)recorte[indS];


							// mesma classe
							if (pval == pvalClass2) {
								area++;

								if (x < xMin)
									xMin = x;

								if (x > xMax)
									xMax = x;

								if (y < yMin)
									yMin = y;

								if (y > yMax)
									yMax = y;



								// instance
								img.at<unsigned char>(y, x) = 255;
							}

							x++;

							if (x > linhaAdd - 1) {
								x = 0;
								y++;
							}

						}



						if (area > 10) {

							boxX = xMin;
							boxY = yMin;
							boxWidth = (xMax - xMin) + 1;
							boxHeight = (yMax - yMin) + 1;


							int idObjeto = valR + (valG * 256) + (valB * 256 * 256);

							//texto += 

							if (indR < pivosDiferentes.Count - 1)
								jsonPanopticTrain->WriteLine("{\"id\": " + idObjeto.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "},");
							
							else
								jsonPanopticTrain->WriteLine("{\"id\": " + idObjeto.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "}");
							
						}



						// calcular vertices instance
						bool isIgnore = false;
						for (int ig = 0; ig < categoriesIgnore.Count; ig++) {

							if (pvalClass == categoriesIgnore[ig]) {
								isIgnore = true;
								break;
							}
						}


						if (!isIgnore && area>10) {

							vector<vector<cv::Point> > contours;

							vector<cv::Vec4i> hierarchy;
							findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

							for (size_t cont = 0; cont < contours.size(); cont++) {

								vector<cv::Point> points = contours[cont];

								if (points.size() >= 6){
									String ^linhaInstance = "";

									for (size_t po = 0; po < points.size(); po++) {

										cv::Point point = points[po];

										if (po < points.size() - 1)
											linhaInstance += point.x.ToString() + "," + point.y.ToString() + ",";
										else
											linhaInstance += point.x.ToString() + "," + point.y.ToString() + "";

									}

									jsonInstanceTrain->WriteLine("{");
									jsonInstanceTrain->WriteLine("\"segmentation\": ");
									jsonInstanceTrain->WriteLine("[");
									jsonInstanceTrain->WriteLine("[" + linhaInstance + "]");
									jsonInstanceTrain->WriteLine("],");
									jsonInstanceTrain->WriteLine("\"id\": " + indContador.ToString() + ", \"image_id\": " + ind.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "");
									jsonInstanceTrain->WriteLine("},");

									indContador++;

									break;
								}
							}



						}

					}


					jsonPanopticTrain->WriteLine("],");

					jsonPanopticTrain->WriteLine("\"file_name\": \"" + ind.ToString() + ".png\", \"image_id\": " + ind.ToString());

					jsonPanopticTrain->WriteLine("},");


					


					databandOutSeq = datasetOutSeq->GetRasterBand(1);
					databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteR, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

					databandOutSeq = datasetOutSeq->GetRasterBand(2);
					databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteG, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

					databandOutSeq = datasetOutSeq->GetRasterBand(3);
					databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteB, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

					
					// converter imagem label para png
					GDALDriver *poDriverLabel;

					poDriverLabel = GetGDALDriverManager()->GetDriverByName("PNG");

					if (poDriverLabel == NULL)
						return;


					GDALDataset *poDstDSLabel;
					poDstDSLabel = poDriverLabel->CreateCopy(nom_arquivo_png, imgLabel->GetDataSet(), FALSE, NULL, NULL, NULL);


					if (poDstDSLabel != NULL)
						GDALClose((GDALDatasetH)poDstDSLabel);


					imgLabel->Close();




					// converter imagem class para png
					GDALDriver *poDriverClass;

					poDriverClass = GetGDALDriverManager()->GetDriverByName("PNG");

					if (poDriverClass == NULL)
						return;


					GDALDataset *poDstDSClass;
					poDstDSClass = poDriverClass->CreateCopy(nom_arquivo_classificada_png, imgClassificada->GetDataSet(), FALSE, NULL, NULL, NULL);


					if (poDstDSClass != NULL)
						GDALClose((GDALDatasetH)poDstDSClass);


					imgClassificada->Close();



					ind++;

				} // fim if valido

				//break;

			} // fim for c


			//break;


			// percent
			Loading::Increment();

		} // fim for r


		jsonPanopticTrain->WriteLine("]");
		jsonPanopticTrain->WriteLine("}");
		jsonPanopticTrain->Close();



		jsonInstanceTrain->WriteLine("],");

		jsonInstanceTrain->WriteLine("\"categories\": [");

		for (c = 0; c < classes->qtde; c++) {
			jsonInstanceTrain->WriteLine("{\"supercategory\": \"city\", \"id\": " + classes->modelos[c].ToString() + ", \"name\": \"" + gcnew String(classes->nomes[c].c_str()) + "\"},");
		}

		jsonInstanceTrain->WriteLine("]");


		jsonInstanceTrain->WriteLine("}");
		jsonInstanceTrain->Close();





		// rois val
		if (qtdAmostrasVal > 0) {

			ind = 1; //variar comeca em 1
			indContador = 1;
			for (r = 0; r < roisVal->size(); r++) {

				ROI proi = roisVal->at(r);

				for (c = 0; c < proi.countValues; c++) {

					col = proi.points[c][0];
					row = proi.points[c][1];

					p1 = row - pmetadeRow;
					p2 = col - pmetadeCol;


					// validar tamanho da imagem
					valido = true;
					if (p2 < 0 || (p2 + linhaAdd) > image->GetCountCols() - 1)
						valido = false;
					else if (p1 < 0 || (p1 + linhaAdd) > image->GetCountRows() - 1)
						valido = false;



					if (valido) {

						// nome imagem
						fileAmostra = pathAmostrasVal + "\\" + ind.ToString() + ".tiff";
						nom_arquivo_amostra = (char*)(void*)Marshal::StringToHGlobalAnsi(fileAmostra);


						fileLabel = pathLabelsVal + "\\" + "img" + "_" + ind.ToString() + ".mem";
						nom_arquivo_label = (char*)(void*)Marshal::StringToHGlobalAnsi(fileLabel);


						filePng = pathLabelsVal + "\\" + ind.ToString() + ".png";
						nom_arquivo_png = (char*)(void*)Marshal::StringToHGlobalAnsi(filePng);


						fileClassificada = pathClassesVal + "\\" + ind.ToString() + ".mem";
						nom_arquivo_classificada = (char*)(void*)Marshal::StringToHGlobalAnsi(fileClassificada);


						fileClassificadaPng = pathClassesVal + "\\" + ind.ToString() + ".png";
						nom_arquivo_classificada_png = (char*)(void*)Marshal::StringToHGlobalAnsi(fileClassificadaPng);




						// criar imagem
						imgAmostra = new TImagem();

						imgAmostra->isBufferRead = false;
						imgAmostra->isBufferWrite = false;

						imgAmostra->Create(nom_arquivo_amostra, TImagem::GTIFF, 1, numBandasCorte, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);


						imgLabel = new TImagem();

						imgLabel->isBufferRead = false;
						imgLabel->isBufferWrite = false;

						imgLabel->Create(nom_arquivo_label, TImagem::MEMORY, 1, 3, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);


						imgClassificada = new TImagem();

						imgClassificada->isBufferRead = false;
						imgClassificada->isBufferWrite = false;

						imgClassificada->Create(nom_arquivo_classificada, TImagem::MEMORY, 1, 1, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);




						// pegar recorte da imagem
						datasetOut = imgAmostra->GetDataSet();

						for (b = 0; b < numBandasCorte; b++) {
							databandIn = datasetIn->GetRasterBand(bandasCorte[b] + 1);
							databandIn->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

							databandOut = datasetOut->GetRasterBand(b + 1);
							databandOut->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);
						}

						imgAmostra->Close();



						// pegar recorte da classificada
						datasetOutClass = imgClassificada->GetDataSet();

						databandInClass = datasetInClass->GetRasterBand(1);
						databandInClass->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorteClass, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOutClass = datasetOutClass->GetRasterBand(1);
						databandOutClass->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteClass, linhaAdd, linhaAdd, GDT_Float32, 0, 0);




						// pegar recorte da sequencial
						datasetOutSeq = imgLabel->GetDataSet();

						databandInSeq = datasetInSeq->GetRasterBand(1);
						databandInSeq->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);



						// calcular R, G, B para cada ponto
						pivosDiferentes.Clear();
						pivosR.Clear();
						pivosG.Clear();
						pivosB.Clear();
						pivosClass.Clear();

						// limpar recortes
						for (indR = 0; indR < size; indR++) {
							recorteR[indR] = 0;
							recorteG[indR] = 0;
							recorteB[indR] = 0;
						}

						for (indR = 0; indR < size; indR++) {

							pval = recorte[indR];
							pvalClass = (int)recorteClass[indR];

							if (pvalClass > 0) {


								valR = fmod(pval, 256);
								aux = floor(pval / 256);
								valG = fmod(aux, 256);
								valB = floor(aux / 256);

								recorteR[indR] = valR;
								recorteG[indR] = valG;
								recorteB[indR] = valB;


								bool existe = false;
								if (pval > 0) {

									for (int j = 0; j < pivosDiferentes.Count; j++) {

										if (pval == pivosDiferentes[j]) {
											existe = true;
											break;
										}

									}


									if (!existe) {
										pivosDiferentes.Add(pval);
										pivosR.Add(valR);
										pivosG.Add(valG);
										pivosB.Add(valB);
										pivosClass.Add(pvalClass);
									}

								}

							}

						}



						// qtd de pivos
						jsonPanopticVal->WriteLine("{");
						jsonPanopticVal->WriteLine("\"segments_info\": ");
						jsonPanopticVal->WriteLine("[");

						for (indR = 0; indR < pivosDiferentes.Count; indR++) {

							valR = pivosR[indR];
							valG = pivosG[indR];
							valB = pivosB[indR];
							pvalClass = pivosClass[indR];
							pval = pivosDiferentes[indR];

							area = 0;
							xMin = 999999;
							xMax = -999999;
							yMin = 999999;
							yMax = -999999;

							x = 0;
							y = 0;


							cv::Mat img = cv::Mat::zeros(linhaAdd, linhaAdd, CV_8UC1);

							for (int indS = 0; indS < size; indS++) {

								pvalClass2 = (int)recorte[indS];


								// mesma classe
								if (pval == pvalClass2) {
									area++;

									if (x < xMin)
										xMin = x;

									if (x > xMax)
										xMax = x;

									if (y < yMin)
										yMin = y;

									if (y > yMax)
										yMax = y;



									// instance
									img.at<unsigned char>(y, x) = 255;
								}

								x++;

								if (x > linhaAdd - 1) {
									x = 0;
									y++;
								}

							}



							boxX = xMin;
							boxY = yMin;
							boxWidth = (xMax - xMin) + 1;
							boxHeight = (yMax - yMin) + 1;


							int idObjeto = valR + (valG * 256) + (valB * 256 * 256);

							if (indR < pivosDiferentes.Count - 1){
								if (area > 10){
									jsonPanopticVal->WriteLine("{\"id\": " + idObjeto.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "},");
								}
							}
							else{
								if (area > 10){
									jsonPanopticVal->WriteLine("{\"id\": " + idObjeto.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "}");
								}
							}



							// calcular vertices instance
							bool isIgnore = false;
							for (int ig = 0; ig < categoriesIgnore.Count; ig++) {

								if (pvalClass == categoriesIgnore[ig]) {
									isIgnore = true;
									break;
								}
							}


							if (!isIgnore && area > 10) {

								vector<vector<cv::Point> > contours;

								vector<cv::Vec4i> hierarchy;
								findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

								for (size_t cont = 0; cont < contours.size(); cont++) {

									vector<cv::Point> points = contours[cont];

									if (points.size() >= 6){
										String ^linhaInstance = "";

										for (size_t po = 0; po < points.size(); po++) {

											cv::Point point = points[po];

											if (po < points.size() - 1)
												linhaInstance += point.x.ToString() + "," + point.y.ToString() + ",";
											else
												linhaInstance += point.x.ToString() + "," + point.y.ToString() + "";

										}

										jsonInstanceVal->WriteLine("{");
										jsonInstanceVal->WriteLine("\"segmentation\": ");
										jsonInstanceVal->WriteLine("[");
										jsonInstanceVal->WriteLine("[" + linhaInstance + "]");
										jsonInstanceVal->WriteLine("],");
										jsonInstanceVal->WriteLine("\"id\": " + indContador.ToString() + ", \"image_id\": " + ind.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "");
										jsonInstanceVal->WriteLine("},");

										indContador++;

										//break;
									}
								}



							}

						}

						jsonPanopticVal->WriteLine("],");

						jsonPanopticVal->WriteLine("\"file_name\": \"" + ind.ToString() + ".png\", \"image_id\": " + ind.ToString());

						jsonPanopticVal->WriteLine("},");





						databandOutSeq = datasetOutSeq->GetRasterBand(1);
						databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteR, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOutSeq = datasetOutSeq->GetRasterBand(2);
						databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteG, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOutSeq = datasetOutSeq->GetRasterBand(3);
						databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteB, linhaAdd, linhaAdd, GDT_Float32, 0, 0);





						// converter imagem label para png
						GDALDriver *poDriverLabel;

						poDriverLabel = GetGDALDriverManager()->GetDriverByName("PNG");

						if (poDriverLabel == NULL)
							return;


						GDALDataset *poDstDSLabel;
						poDstDSLabel = poDriverLabel->CreateCopy(nom_arquivo_png, imgLabel->GetDataSet(), FALSE, NULL, NULL, NULL);


						if (poDstDSLabel != NULL)
							GDALClose((GDALDatasetH)poDstDSLabel);


						imgLabel->Close();




						// converter imagem class para png
						GDALDriver *poDriverClass;

						poDriverClass = GetGDALDriverManager()->GetDriverByName("PNG");

						if (poDriverClass == NULL)
							return;


						GDALDataset *poDstDSClass;
						poDstDSClass = poDriverClass->CreateCopy(nom_arquivo_classificada_png, imgClassificada->GetDataSet(), FALSE, NULL, NULL, NULL);


						if (poDstDSClass != NULL)
							GDALClose((GDALDatasetH)poDstDSClass);


						imgClassificada->Close();



						ind++;

					} // fim if valido

					//break;

					

				} // fim for c

				//break;

				// percent
				Loading::Increment();

			} // fim for r



			jsonPanopticVal->WriteLine("]");
			jsonPanopticVal->WriteLine("}");
			jsonPanopticVal->Close();



			jsonInstanceVal->WriteLine("],");

			jsonInstanceVal->WriteLine("\"categories\": [");

			for (c = 0; c < classes->qtde; c++) {
				jsonInstanceVal->WriteLine("{\"supercategory\": \"city\", \"id\": " + classes->modelos[c].ToString() + ", \"name\": \"" + gcnew String(classes->nomes[c].c_str()) + "\"},");
			}

			jsonInstanceVal->WriteLine("]");


			jsonInstanceVal->WriteLine("}");
			jsonInstanceVal->Close();

		}




		// rois test
		if (qtdAmostrasTest > 0) {

			ind = 1; //variar comeca em 1
			indContador = 1;
			for (r = 0; r < roisTest->size(); r++) {

				ROI proi = roisTest->at(r);

				for (c = 0; c < proi.countValues; c++) {

					col = proi.points[c][0];
					row = proi.points[c][1];

					p1 = row - pmetadeRow;
					p2 = col - pmetadeCol;


					// validar tamanho da imagem
					valido = true;
					if (p2 < 0 || (p2 + linhaAdd) > image->GetCountCols() - 1)
						valido = false;
					else if (p1 < 0 || (p1 + linhaAdd) > image->GetCountRows() - 1)
						valido = false;



					if (valido) {

						// nome imagem
						fileAmostra = pathAmostrasTest + "\\" + ind.ToString() + ".tiff";
						nom_arquivo_amostra = (char*)(void*)Marshal::StringToHGlobalAnsi(fileAmostra);


						fileLabel = pathLabelsTest + "\\" + "img" + "_" + ind.ToString() + ".mem";
						nom_arquivo_label = (char*)(void*)Marshal::StringToHGlobalAnsi(fileLabel);


						filePng = pathLabelsTest + "\\" + ind.ToString() + ".png";
						nom_arquivo_png = (char*)(void*)Marshal::StringToHGlobalAnsi(filePng);


						fileClassificada = pathClassesTest + "\\" + ind.ToString() + ".mem";
						nom_arquivo_classificada = (char*)(void*)Marshal::StringToHGlobalAnsi(fileClassificada);


						fileClassificadaPng = pathClassesTest + "\\" + ind.ToString() + ".png";
						nom_arquivo_classificada_png = (char*)(void*)Marshal::StringToHGlobalAnsi(fileClassificadaPng);




						// criar imagem
						imgAmostra = new TImagem();

						imgAmostra->isBufferRead = false;
						imgAmostra->isBufferWrite = false;

						imgAmostra->Create(nom_arquivo_amostra, TImagem::GTIFF, 1, numBandasCorte, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);


						imgLabel = new TImagem();

						imgLabel->isBufferRead = false;
						imgLabel->isBufferWrite = false;

						imgLabel->Create(nom_arquivo_label, TImagem::MEMORY, 1, 3, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);


						imgClassificada = new TImagem();

						imgClassificada->isBufferRead = false;
						imgClassificada->isBufferWrite = false;

						imgClassificada->Create(nom_arquivo_classificada, TImagem::MEMORY, 1, 1, linhaAdd, linhaAdd, NULL, NULL, NULL, NULL, NULL);




						// pegar recorte da imagem
						datasetOut = imgAmostra->GetDataSet();

						for (b = 0; b < numBandasCorte; b++) {
							databandIn = datasetIn->GetRasterBand(bandasCorte[b] + 1);
							databandIn->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

							databandOut = datasetOut->GetRasterBand(b + 1);
							databandOut->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);
						}

						imgAmostra->Close();



						// pegar recorte da classificada
						datasetOutClass = imgClassificada->GetDataSet();

						databandInClass = datasetInClass->GetRasterBand(1);
						databandInClass->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorteClass, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOutClass = datasetOutClass->GetRasterBand(1);
						databandOutClass->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteClass, linhaAdd, linhaAdd, GDT_Float32, 0, 0);




						// pegar recorte da sequencial
						datasetOutSeq = imgLabel->GetDataSet();

						databandInSeq = datasetInSeq->GetRasterBand(1);
						databandInSeq->RasterIO(GF_Read, p2, p1, linhaAdd, linhaAdd, recorte, linhaAdd, linhaAdd, GDT_Float32, 0, 0);



						// calcular R, G, B para cada ponto
						pivosDiferentes.Clear();
						pivosR.Clear();
						pivosG.Clear();
						pivosB.Clear();
						pivosClass.Clear();

						// limpar recortes
						for (indR = 0; indR < size; indR++) {
							recorteR[indR] = 0;
							recorteG[indR] = 0;
							recorteB[indR] = 0;
						}

						for (indR = 0; indR < size; indR++) {

							pval = recorte[indR];
							pvalClass = (int)recorteClass[indR];

							if (pvalClass > 0) {


								valR = fmod(pval, 256);
								aux = floor(pval / 256);
								valG = fmod(aux, 256);
								valB = floor(aux / 256);

								recorteR[indR] = valR;
								recorteG[indR] = valG;
								recorteB[indR] = valB;


								bool existe = false;
								if (pval > 0) {

									for (int j = 0; j < pivosDiferentes.Count; j++) {

										if (pval == pivosDiferentes[j]) {
											existe = true;
											break;
										}

									}


									if (!existe) {
										pivosDiferentes.Add(pval);
										pivosR.Add(valR);
										pivosG.Add(valG);
										pivosB.Add(valB);
										pivosClass.Add(pvalClass);
									}

								}

							}

						}



						// qtd de pivos
						jsonPanopticTest->WriteLine("{");
						jsonPanopticTest->WriteLine("\"segments_info\": ");
						jsonPanopticTest->WriteLine("[");

						for (indR = 0; indR < pivosDiferentes.Count; indR++) {

							valR = pivosR[indR];
							valG = pivosG[indR];
							valB = pivosB[indR];
							pvalClass = pivosClass[indR];
							pval = pivosDiferentes[indR];

							area = 0;
							xMin = 999999;
							xMax = -999999;
							yMin = 999999;
							yMax = -999999;

							x = 0;
							y = 0;


							cv::Mat img = cv::Mat::zeros(linhaAdd, linhaAdd, CV_8UC1);

							for (int indS = 0; indS < size; indS++) {

								pvalClass2 = (int)recorte[indS];


								// mesma classe
								if (pval == pvalClass2) {
									area++;

									if (x < xMin)
										xMin = x;

									if (x > xMax)
										xMax = x;

									if (y < yMin)
										yMin = y;

									if (y > yMax)
										yMax = y;



									// instance
									img.at<unsigned char>(y, x) = 255;
								}

								x++;

								if (x > linhaAdd - 1) {
									x = 0;
									y++;
								}

							}



							boxX = xMin;
							boxY = yMin;
							boxWidth = (xMax - xMin) + 1;
							boxHeight = (yMax - yMin) + 1;


							int idObjeto = valR + (valG * 256) + (valB * 256 * 256);

							if (indR < pivosDiferentes.Count - 1){
								if (area > 10){
									jsonPanopticTest->WriteLine("{\"id\": " + idObjeto.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "},");
								}
							}
							else{
								if (area > 10){
									jsonPanopticTest->WriteLine("{\"id\": " + idObjeto.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "}");
								}
							}



							// calcular vertices instance
							bool isIgnore = false;
							for (int ig = 0; ig < categoriesIgnore.Count; ig++) {

								if (pvalClass == categoriesIgnore[ig]) {
									isIgnore = true;
									break;
								}
							}


							if (!isIgnore && area > 10) {

								vector<vector<cv::Point> > contours;

								vector<cv::Vec4i> hierarchy;
								findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

								for (size_t cont = 0; cont < contours.size(); cont++) {

									vector<cv::Point> points = contours[cont];

									if (points.size() >= 6){
										String ^linhaInstance = "";

										for (size_t po = 0; po < points.size(); po++) {

											cv::Point point = points[po];

											if (po < points.size() - 1)
												linhaInstance += point.x.ToString() + "," + point.y.ToString() + ",";
											else
												linhaInstance += point.x.ToString() + "," + point.y.ToString() + "";

										}

										jsonInstanceTest->WriteLine("{");
										jsonInstanceTest->WriteLine("\"segmentation\": ");
										jsonInstanceTest->WriteLine("[");
										jsonInstanceTest->WriteLine("[" + linhaInstance + "]");
										jsonInstanceTest->WriteLine("],");
										jsonInstanceTest->WriteLine("\"id\": " + indContador.ToString() + ", \"image_id\": " + ind.ToString() + ", \"category_id\": " + pvalClass.ToString() + ", \"iscrowd\": 0, \"bbox\": [" + boxX.ToString() + ", " + boxY.ToString() + ", " + boxWidth.ToString() + ", " + boxHeight.ToString() + "], \"area\": " + area.ToString() + "");
										jsonInstanceTest->WriteLine("},");

										indContador++;

										//break;
									}
								}



							}

						}

						jsonPanopticTest->WriteLine("],");

						jsonPanopticTest->WriteLine("\"file_name\": \"" + ind.ToString() + ".png\", \"image_id\": " + ind.ToString());

						jsonPanopticTest->WriteLine("},");





						databandOutSeq = datasetOutSeq->GetRasterBand(1);
						databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteR, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOutSeq = datasetOutSeq->GetRasterBand(2);
						databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteG, linhaAdd, linhaAdd, GDT_Float32, 0, 0);

						databandOutSeq = datasetOutSeq->GetRasterBand(3);
						databandOutSeq->RasterIO(GF_Write, 0, 0, linhaAdd, linhaAdd, recorteB, linhaAdd, linhaAdd, GDT_Float32, 0, 0);





						// converter imagem label para png
						GDALDriver *poDriverLabel;

						poDriverLabel = GetGDALDriverManager()->GetDriverByName("PNG");

						if (poDriverLabel == NULL)
							return;


						GDALDataset *poDstDSLabel;
						poDstDSLabel = poDriverLabel->CreateCopy(nom_arquivo_png, imgLabel->GetDataSet(), FALSE, NULL, NULL, NULL);


						if (poDstDSLabel != NULL)
							GDALClose((GDALDatasetH)poDstDSLabel);


						imgLabel->Close();




						// converter imagem class para png
						GDALDriver *poDriverClass;

						poDriverClass = GetGDALDriverManager()->GetDriverByName("PNG");

						if (poDriverClass == NULL)
							return;


						GDALDataset *poDstDSClass;
						poDstDSClass = poDriverClass->CreateCopy(nom_arquivo_classificada_png, imgClassificada->GetDataSet(), FALSE, NULL, NULL, NULL);


						if (poDstDSClass != NULL)
							GDALClose((GDALDatasetH)poDstDSClass);


						imgClassificada->Close();



						ind++;

					} // fim if valido

					//break;



				} // fim for c

				//break;

				// percent
				Loading::Increment();

			} // fim for r



			jsonPanopticTest->WriteLine("]");
			jsonPanopticTest->WriteLine("}");
			jsonPanopticTest->Close();



			jsonInstanceTest->WriteLine("],");

			jsonInstanceTest->WriteLine("\"categories\": [");

			for (c = 0; c < classes->qtde; c++) {
				jsonInstanceTest->WriteLine("{\"supercategory\": \"city\", \"id\": " + classes->modelos[c].ToString() + ", \"name\": \"" + gcnew String(classes->nomes[c].c_str()) + "\"},");
			}

			jsonInstanceTest->WriteLine("]");


			jsonInstanceTest->WriteLine("}");
			jsonInstanceTest->Close();

		}


		Loading::Stop();


		MessageBox::Show("Amostras salvas com sucesso");


		// destravar form
		this->Enabled = true;
		

	}

}