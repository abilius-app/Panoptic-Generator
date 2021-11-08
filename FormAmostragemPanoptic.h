#pragma once

#include "TImagem.h"
#include "ImageLib.h"

#include "Rois.h"

namespace Abilio {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for FormAmostragemPanoptic
	/// </summary>
	public ref class FormAmostragemPanoptic : public System::Windows::Forms::Form
	{
	public:
		Form ^formPrincipal;

		TImagem *image, *imageClassificada, *imageSequencial;
		vector<ROI> *roisTrain, *roisVal, *roisTest;
		int qtdAmostrasTrain, qtdAmostrasVal, qtdAmostrasTest;

		FormAmostragemPanoptic(void)
		{
			InitializeComponent();


			roisTrain = NULL;
			roisVal = NULL;
			roisTest = NULL;

			image = NULL;
			imageClassificada = NULL;
			imageSequencial = NULL;

			qtdAmostrasTrain = 0;
			qtdAmostrasVal = 0;
			qtdAmostrasTest = 0;

			gbImagem->AllowDrop = true;
			gbClassificada->AllowDrop = true;
			gbSequencial->AllowDrop = true;
		}

		void ImportarShapeTrain();
		void ImportarShapeVal();
		void ImportarShapeTest();
		void Aplicar();

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~FormAmostragemPanoptic()
		{
			if (components)
			{
				delete components;
			}
		}


	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Label^  label7;


	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Label^  label4;

	private: System::Windows::Forms::GroupBox^  gbSequencial;
	private: System::Windows::Forms::Label^  label3;

	private: System::Windows::Forms::ListBox^  listBoxBandas;

	private: System::Windows::Forms::FolderBrowserDialog^  folderBrowserDialog1;

	private: System::Windows::Forms::GroupBox^  groupBox1;


	private: System::Windows::Forms::OpenFileDialog^  dialogImportarShape;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::Label^  lblQtdAmostras;
	private: System::Windows::Forms::Button^  btnImportar;
	private: System::Windows::Forms::GroupBox^  gbImagem;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::GroupBox^  gbClassificada;

	private: System::Windows::Forms::Label^  label2;



	private: System::Windows::Forms::GroupBox^  gbParametros;
	private: System::Windows::Forms::NumericUpDown^  numJanColunas;

	private: System::Windows::Forms::NumericUpDown^  numJanLinhas;

	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  label6;


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->gbImagem = (gcnew System::Windows::Forms::GroupBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->gbClassificada = (gcnew System::Windows::Forms::GroupBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->gbParametros = (gcnew System::Windows::Forms::GroupBox());
			this->numJanColunas = (gcnew System::Windows::Forms::NumericUpDown());
			this->numJanLinhas = (gcnew System::Windows::Forms::NumericUpDown());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->btnImportar = (gcnew System::Windows::Forms::Button());
			this->lblQtdAmostras = (gcnew System::Windows::Forms::Label());
			this->dialogImportarShape = (gcnew System::Windows::Forms::OpenFileDialog());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->listBoxBandas = (gcnew System::Windows::Forms::ListBox());
			this->folderBrowserDialog1 = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->gbSequencial = (gcnew System::Windows::Forms::GroupBox());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->gbImagem->SuspendLayout();
			this->gbClassificada->SuspendLayout();
			this->gbParametros->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numJanColunas))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numJanLinhas))->BeginInit();
			this->groupBox2->SuspendLayout();
			this->groupBox1->SuspendLayout();
			this->gbSequencial->SuspendLayout();
			this->groupBox3->SuspendLayout();
			this->groupBox4->SuspendLayout();
			this->SuspendLayout();
			// 
			// gbImagem
			// 
			this->gbImagem->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->gbImagem->Controls->Add(this->label1);
			this->gbImagem->Location = System::Drawing::Point(3, 6);
			this->gbImagem->Name = L"gbImagem";
			this->gbImagem->Size = System::Drawing::Size(357, 49);
			this->gbImagem->TabIndex = 23;
			this->gbImagem->TabStop = false;
			this->gbImagem->Text = L"Image";
			this->gbImagem->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &FormAmostragemPanoptic::gbImagem_DragDrop);
			this->gbImagem->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &FormAmostragemPanoptic::gbImagem_DragEnter);
			this->gbImagem->Enter += gcnew System::EventHandler(this, &FormAmostragemPanoptic::gbImagem_Enter);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(11, 24);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(104, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Select original image";
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button1->Location = System::Drawing::Point(247, 575);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(101, 23);
			this->button1->TabIndex = 25;
			this->button1->Text = L"Apply";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &FormAmostragemPanoptic::button1_Click);
			// 
			// button3
			// 
			this->button3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->button3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button3->Location = System::Drawing::Point(162, 575);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(75, 23);
			this->button3->TabIndex = 26;
			this->button3->Text = L"Cancel";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &FormAmostragemPanoptic::button3_Click);
			// 
			// gbClassificada
			// 
			this->gbClassificada->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->gbClassificada->Controls->Add(this->label2);
			this->gbClassificada->Location = System::Drawing::Point(3, 59);
			this->gbClassificada->Name = L"gbClassificada";
			this->gbClassificada->Size = System::Drawing::Size(357, 49);
			this->gbClassificada->TabIndex = 27;
			this->gbClassificada->TabStop = false;
			this->gbClassificada->Text = L"Semantic Annotation";
			this->gbClassificada->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &FormAmostragemPanoptic::gbClassificada_DragDrop);
			this->gbClassificada->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &FormAmostragemPanoptic::gbClassificada_DragEnter);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(11, 24);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(135, 13);
			this->label2->TabIndex = 0;
			this->label2->Text = L"Select semantic annotation";
			// 
			// gbParametros
			// 
			this->gbParametros->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->gbParametros->Controls->Add(this->numJanColunas);
			this->gbParametros->Controls->Add(this->numJanLinhas);
			this->gbParametros->Controls->Add(this->label5);
			this->gbParametros->Controls->Add(this->label6);
			this->gbParametros->Location = System::Drawing::Point(3, 325);
			this->gbParametros->Name = L"gbParametros";
			this->gbParametros->Size = System::Drawing::Size(357, 50);
			this->gbParametros->TabIndex = 29;
			this->gbParametros->TabStop = false;
			this->gbParametros->Text = L"Frame Size";
			// 
			// numJanColunas
			// 
			this->numJanColunas->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
			this->numJanColunas->Location = System::Drawing::Point(235, 21);
			this->numJanColunas->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 9999, 0, 0, 0 });
			this->numJanColunas->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numJanColunas->Name = L"numJanColunas";
			this->numJanColunas->Size = System::Drawing::Size(56, 20);
			this->numJanColunas->TabIndex = 14;
			this->numJanColunas->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			// 
			// numJanLinhas
			// 
			this->numJanLinhas->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
			this->numJanLinhas->Location = System::Drawing::Point(87, 19);
			this->numJanLinhas->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 9999, 0, 0, 0 });
			this->numJanLinhas->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numJanLinhas->Name = L"numJanLinhas";
			this->numJanLinhas->Size = System::Drawing::Size(56, 20);
			this->numJanLinhas->TabIndex = 13;
			this->numJanLinhas->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(35, 21);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(41, 13);
			this->label5->TabIndex = 10;
			this->label5->Text = L"Height:";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(176, 23);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(35, 13);
			this->label6->TabIndex = 11;
			this->label6->Text = L"Width";
			// 
			// groupBox2
			// 
			this->groupBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->groupBox2->Controls->Add(this->btnImportar);
			this->groupBox2->Controls->Add(this->lblQtdAmostras);
			this->groupBox2->Location = System::Drawing::Point(3, 165);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(357, 49);
			this->groupBox2->TabIndex = 31;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Shape Train";
			// 
			// btnImportar
			// 
			this->btnImportar->Enabled = false;
			this->btnImportar->Location = System::Drawing::Point(270, 19);
			this->btnImportar->Name = L"btnImportar";
			this->btnImportar->Size = System::Drawing::Size(75, 23);
			this->btnImportar->TabIndex = 1;
			this->btnImportar->Text = L"Import";
			this->btnImportar->UseVisualStyleBackColor = true;
			this->btnImportar->Click += gcnew System::EventHandler(this, &FormAmostragemPanoptic::btnImportar_Click);
			// 
			// lblQtdAmostras
			// 
			this->lblQtdAmostras->AutoSize = true;
			this->lblQtdAmostras->Location = System::Drawing::Point(11, 24);
			this->lblQtdAmostras->Name = L"lblQtdAmostras";
			this->lblQtdAmostras->Size = System::Drawing::Size(77, 13);
			this->lblQtdAmostras->TabIndex = 0;
			this->lblQtdAmostras->Text = L"Import shape 1";
			// 
			// dialogImportarShape
			// 
			this->dialogImportarShape->FileName = L"dialogAmostras";
			this->dialogImportarShape->Filter = L"Shape|*.shp";
			// 
			// groupBox1
			// 
			this->groupBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->groupBox1->Controls->Add(this->listBoxBandas);
			this->groupBox1->Location = System::Drawing::Point(3, 379);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(357, 181);
			this->groupBox1->TabIndex = 32;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Number of channels";
			// 
			// listBoxBandas
			// 
			this->listBoxBandas->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->listBoxBandas->FormattingEnabled = true;
			this->listBoxBandas->Location = System::Drawing::Point(6, 16);
			this->listBoxBandas->Name = L"listBoxBandas";
			this->listBoxBandas->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
			this->listBoxBandas->Size = System::Drawing::Size(345, 160);
			this->listBoxBandas->TabIndex = 18;
			// 
			// gbSequencial
			// 
			this->gbSequencial->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->gbSequencial->Controls->Add(this->label3);
			this->gbSequencial->Location = System::Drawing::Point(3, 112);
			this->gbSequencial->Name = L"gbSequencial";
			this->gbSequencial->Size = System::Drawing::Size(357, 49);
			this->gbSequencial->TabIndex = 33;
			this->gbSequencial->TabStop = false;
			this->gbSequencial->Text = L"Panoptic Annotation";
			this->gbSequencial->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &FormAmostragemPanoptic::gbSequencial_DragDrop);
			this->gbSequencial->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &FormAmostragemPanoptic::gbSequencial_DragEnter);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(11, 24);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(134, 13);
			this->label3->TabIndex = 0;
			this->label3->Text = L"Select panoptic annotation";
			// 
			// groupBox3
			// 
			this->groupBox3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->groupBox3->Controls->Add(this->button2);
			this->groupBox3->Controls->Add(this->label4);
			this->groupBox3->Location = System::Drawing::Point(3, 218);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(357, 49);
			this->groupBox3->TabIndex = 34;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"Shape Val";
			// 
			// button2
			// 
			this->button2->Enabled = false;
			this->button2->Location = System::Drawing::Point(270, 19);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(75, 23);
			this->button2->TabIndex = 1;
			this->button2->Text = L"Import";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &FormAmostragemPanoptic::button2_Click);
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(11, 24);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(77, 13);
			this->label4->TabIndex = 0;
			this->label4->Text = L"Import shape 2";
			// 
			// groupBox4
			// 
			this->groupBox4->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->groupBox4->Controls->Add(this->button4);
			this->groupBox4->Controls->Add(this->label7);
			this->groupBox4->Location = System::Drawing::Point(3, 271);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(357, 49);
			this->groupBox4->TabIndex = 35;
			this->groupBox4->TabStop = false;
			this->groupBox4->Text = L"Shape Test";
			// 
			// button4
			// 
			this->button4->Enabled = false;
			this->button4->Location = System::Drawing::Point(270, 19);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(75, 23);
			this->button4->TabIndex = 1;
			this->button4->Text = L"Import";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &FormAmostragemPanoptic::button4_Click);
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(11, 24);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(77, 13);
			this->label7->TabIndex = 0;
			this->label7->Text = L"Import shape 3";
			// 
			// FormAmostragemPanoptic
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(360, 610);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->groupBox3);
			this->Controls->Add(this->gbSequencial);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->gbParametros);
			this->Controls->Add(this->gbClassificada);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->button3);
			this->Controls->Add(this->gbImagem);
			this->Name = L"FormAmostragemPanoptic";
			this->Text = L"Panoptic Annotation Generator";
			this->Load += gcnew System::EventHandler(this, &FormAmostragemPanoptic::FormAmostragemPanoptic_Load);
			this->gbImagem->ResumeLayout(false);
			this->gbImagem->PerformLayout();
			this->gbClassificada->ResumeLayout(false);
			this->gbClassificada->PerformLayout();
			this->gbParametros->ResumeLayout(false);
			this->gbParametros->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numJanColunas))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numJanLinhas))->EndInit();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->groupBox1->ResumeLayout(false);
			this->gbSequencial->ResumeLayout(false);
			this->gbSequencial->PerformLayout();
			this->groupBox3->ResumeLayout(false);
			this->groupBox3->PerformLayout();
			this->groupBox4->ResumeLayout(false);
			this->groupBox4->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion

	private: System::Void gbImagem_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

		if (e->Data->GetDataPresent("System.Windows.Forms.TreeNode", false))
		{

			TreeNode ^node = (TreeNode ^)e->Data->GetData("System.Windows.Forms.TreeNode");

			// procurar o index da imagem aberta
			while (node->Parent != nullptr) {

				node = node->Parent;
			}

			int indImagem = node->Index;



			image = ImageLib::GetImageOpen(indImagem);

			String ^nome = gcnew String(image->GetName().c_str());

			label1->Text = nome;

			btnImportar->Enabled = true;
			button2->Enabled = true;
			button4->Enabled = true;


			listBoxBandas->Items->Clear();
			for (int bands = 0; bands < image->GetCountBands(); bands++)
				listBoxBandas->Items->Add(gcnew String(image->GetBandName(0, bands)));

		}

	}

	private: System::Void gbImagem_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

		if (e->Data->GetDataPresent("System.Windows.Forms.TreeNode", false)) {

			e->Effect = e->AllowedEffect;
		}

	}

	private: System::Void gbClassificada_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

		if (e->Data->GetDataPresent("System.Windows.Forms.TreeNode", false))
		{

			TreeNode ^node = (TreeNode ^)e->Data->GetData("System.Windows.Forms.TreeNode");

			// procurar o index da imagem aberta
			while (node->Parent != nullptr) {

				node = node->Parent;
			}

			int indImagem = node->Index;



			imageClassificada = ImageLib::GetImageOpen(indImagem);

			if (imageClassificada->GetClasses() == NULL) {
				MessageBox::Show("Adicione uma imagem classificada!");
				imageClassificada = NULL;
				return;
			}

			String ^nome = gcnew String(imageClassificada->GetName().c_str());

			label2->Text = nome;

		}

	}

	private: System::Void gbClassificada_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

		if (e->Data->GetDataPresent("System.Windows.Forms.TreeNode", false)) {

			e->Effect = e->AllowedEffect;
		}

	}

	private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {

		Close();

	}

	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

		Aplicar();

	}

	private: System::Void btnImportar_Click(System::Object^  sender, System::EventArgs^  e) {

		ImportarShapeTrain();

	}
	private: System::Void FormAmostragemPanoptic_Load(System::Object^  sender, System::EventArgs^  e) {
	}
	private: System::Void gbSequencial_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

		if (e->Data->GetDataPresent("System.Windows.Forms.TreeNode", false))
		{

			TreeNode ^node = (TreeNode ^)e->Data->GetData("System.Windows.Forms.TreeNode");

			// procurar o index da imagem aberta
			while (node->Parent != nullptr) {

				node = node->Parent;
			}

			int indImagem = node->Index;



			imageSequencial = ImageLib::GetImageOpen(indImagem);

			/*if (imageClassificada->GetClasses()->qtde == 0) {
			MessageBox::Show("Adicione uma imagem classificada!");
			imageClassificada = NULL;
			return;
			}*/

			String ^nome = gcnew String(imageSequencial->GetName().c_str());

			label3->Text = nome;

		}

	}
	private: System::Void gbSequencial_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

		if (e->Data->GetDataPresent("System.Windows.Forms.TreeNode", false)) {

			e->Effect = e->AllowedEffect;
		}

	}
	private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {

		ImportarShapeVal();
	}
private: System::Void gbImagem_Enter(System::Object^  sender, System::EventArgs^  e) {
}
private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {

	ImportarShapeTest();

}
};
}
