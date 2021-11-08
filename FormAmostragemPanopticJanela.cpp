#include "FormAmostragemPanopticJanela.h"

#include "FormPrincipal.h"

#include "ThreadParameters.h"
#include "ThreadImage.h"

#include "ogrsf_frmts.h"

#include <stdio.h> 

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

namespace Abilio {

	void FormAmostragemPanopticJanela::Aplicar()
	{
		TImagem *imgAmostra, *imgLabel, *imgPng, *imgClassificada;
		TClasses *classes;
		int row, col, time, b, indRow, indCol, region, ind, indRoi, pjlin, pjcol, r, c, indPonto, indR, size, aux, aux2;
		int linhaAdd, p1;
		int colunaAdd, p2, indImg;
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
		int step;
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
		step = (int)numJanPasso->Value;


		// salvar como
		if (folderBrowserDialog1->ShowDialog() != System::Windows::Forms::DialogResult::OK)
			return;


		// criar arquivo
		String ^path = folderBrowserDialog1->SelectedPath;



		// montar classes
		classes = imageClassificada->GetClasses();


		int qtdPassoRow = (int) (image->GetCountRows() - linhaAdd) / step;
		int qtdPassoCol = (int)(image->GetCountCols() - colunaAdd) / step;

		safe_cast<FormPrincipal ^>(this->formPrincipal)->Loading(qtdPassoRow + qtdPassoCol);



		// travar form
		this->Enabled = false;



		// criar pastas
		String ^pathAnnotations = path + "\\" + "annotations";
		String ^pathAmostrasVal = path + "\\" + "image_val";
		String ^pathLabelsVal = path + "\\" + "panoptic_val";
		String ^pathClassesVal = path + "\\" + "class_val";


		std::string nom_path_annotations = (char*)(void*)Marshal::StringToHGlobalAnsi(pathAnnotations);
		std::string nom_path_amostras_val = (char*)(void*)Marshal::StringToHGlobalAnsi(pathAmostrasVal);
		std::string nom_path_labes_val = (char*)(void*)Marshal::StringToHGlobalAnsi(pathLabelsVal);
		std::string nom_path_classes_val = (char*)(void*)Marshal::StringToHGlobalAnsi(pathClassesVal);

		mkdir(nom_path_annotations.c_str());
		mkdir(nom_path_amostras_val.c_str());
		mkdir(nom_path_labes_val.c_str());
		mkdir(nom_path_classes_val.c_str());



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



		// annotations val
		StreamWriter ^jsonPanopticVal = gcnew StreamWriter(pathAnnotations + "\\" + "panoptic_val.json");
		StreamWriter ^jsonInstanceVal = gcnew StreamWriter(pathAnnotations + "\\" + "instance_val.json");


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
		for (row = 0; row < image->GetCountRows(); row += step) {

			for (col = 0; col < image->GetCountCols(); col += step) {

				p1 = row;
				p2 = col;


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



		categoriesIgnore.Clear();
		categoriesIgnore.Add(0);
		categoriesIgnore.Add(1);
		categoriesIgnore.Add(2);
		categoriesIgnore.Add(3);





		ind = 1; //variar comeca em 1
		indContador = 1;
		for (row = 0; row < image->GetCountRows(); row += step) {

			for (col = 0; col < image->GetCountCols(); col += step) {

				p1 = row;
				p2 = col;


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
					for (indR = 0; indR < size; indR++) {

						pval = recorte[indR];
						pvalClass = (int)recorteClass[indR];


						if (pval == 0) { // background
							valR = 0;
							valG = 0;
							valB = 0;
						}
						else { // classes

							if (pval < 256) {
								valR = pval;
								valG = 0;
								valB = 0;
							}
							else if (pval >= 256 && pval < 65536) {

								aux = floor(pval / 256);

								valR = aux;
								valG = (pval - (aux * 256)) + 1;
								valB = 0;

							}
							else {

								aux = floor(pval / 256);
								aux2 = floor(aux / 256);

								if (aux < 256) {
									valR = aux - 256;
									valG = (pval - (aux * 256)) + 1;
									valB = 0;
								}
								else {
									valR = aux2 - 256;
									valG = (pval - (aux * 256)) + 1;
									valB = 0;
								}

							}

						}


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

									break;
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

				// percent
				Loading::Increment();

			} // fim for col


			

		} // fim for row



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


		Loading::Stop();


		MessageBox::Show("Amostras salvas com sucesso");


		// destravar form
		this->Enabled = true;


	}

}