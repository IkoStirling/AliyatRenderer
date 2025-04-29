#include <iostream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-image.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <regex>
#include <sstream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <poppler/cpp/poppler-page-renderer.h>
#include <filesystem>

std::string utf8_to_gbk(const std::string& utf8_str) {
	int len_wide = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
	if (len_wide <= 0) return "";

	std::wstring wide_str(len_wide, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wide_str[0], len_wide);

	int len_gbk = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (len_gbk <= 0) return "";

	std::string gbk_str(len_gbk, 0);
	WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, &gbk_str[0], len_gbk, nullptr, nullptr);

	return gbk_str;
}

std::string wstring_to_utf8(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

std::wstring utf8_to_wstring(const std::string& utf8) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(utf8);
}

std::wstring OpenFileExplorerAndGetPath(HWND hwnd = NULL) {
	OPENFILENAMEW ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	// ��ʼ���ṹ��
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;  // �����ھ������ NULL ���޸�����
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = L"Select a File";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrDefExt = NULL;

	if (GetOpenFileNameW(&ofn)) {
		return std::wstring(ofn.lpstrFile);  // ����ѡ����ļ�·��
	}
	return L"";  // �û�ȡ��ѡ��
}

bool isBlankLine(const std::string& line) {
	return std::all_of(line.begin(), line.end(), [](char c) {
		return c == '\r' || c == '\n' || c == ' ' || c == '\t';
		});
}

void sanitizeText(std::string& text) {
	// �滻���������հ��ַ�Ϊ�����ո�
	text.erase(std::unique(text.begin(), text.end(),
		[](char a, char b) { return isspace(a) && isspace(b); }),
		text.end());

	// �Ƴ���ͷ��β�Ŀհ�
	text.erase(0, text.find_first_not_of(" \t\n\r"));
	text.erase(text.find_last_not_of(" \t\n\r") + 1);
}

std::string getTotalText(const std::string& path)
{
	std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(path));
	if (!doc) {
		std::cerr << "�޷��� PDF �ļ�" << std::endl;
		return "";
	}

	int total_pages = doc->pages();
	//total_pages = 1;
	//std::cout << "��ҳ��: " << total_pages << "\n";

	std::string tt;
	for (int i = 0; i < total_pages; ++i) {
		std::unique_ptr<poppler::page> page(doc->create_page(i));
		if (page) {
			auto text = page->text().to_utf8();
			std::string utf8_text(text.data(), text.size());
			tt.append(utf8_text);
		}
	}
	return tt;
}

struct Question {
	int number;
	std::wstring content;
};

struct FormatAnswers {
	std::wstring text;
	std::string imgPath;
};

struct FormatQuestion {
	int number;
	std::wstring question;
	uint8_t answerNum;
	std::vector<FormatAnswers> answers;
	std::vector<std::string> imgPaths;
};



std::vector<Question> SplitModules(const std::wstring& text, std::wregex& reg) {
	std::vector<Question> modules;

	std::wsmatch match;
	std::wstring wide_text = text;
	std::wregex wide_regex(reg);

	std::vector<std::wstring> lines;
	std::wistringstream iss(wide_text);
	std::wstring line;
	while (std::getline(iss, line)) {
		lines.push_back(line);
	}

	std::wstring curModule;
	int curNum = 0;
	bool in_module = false;

	for (size_t i = 0; i < lines.size(); ++i) {
		if (std::regex_search(lines[i], match, wide_regex)) {
			if (in_module)
			{
				modules.push_back({ curNum, curModule });
				curModule.clear();
			}
			curNum++;
			curModule = lines[i];
			in_module = true;
		}
		else if (in_module)
		{
			if (!curModule.empty())
				curModule.append(L"\n");
			curModule.append(lines[i]);
		}
	}

	if (!curModule.empty()) {
		modules.push_back({ curNum, curModule });
	}

	return modules;
}

std::vector<Question> SplitSubModules(const std::wstring& text, std::wregex& reg) {
	std::vector<Question> submodules;

	std::wsmatch match;
	std::wstring wide_text = text;
	std::wregex wide_regex(reg);

	std::vector<std::wstring> lines;
	std::wistringstream iss(wide_text);
	std::wstring line;
	while (std::getline(iss, line)) {
		lines.push_back(line);
	}

	std::wstring curSubmodule;
	int curNum = 0;
	bool in_module = false;

	for (size_t i = 0; i < lines.size(); ++i) {
		if (std::regex_search(lines[i], match, wide_regex)) {
			if (in_module)
			{
				submodules.push_back({ curNum, curSubmodule });
				curSubmodule.clear();
			}
			curNum++;
			curSubmodule = lines[i];
			in_module = true;
		}
		else if (in_module)
		{
			if (!curSubmodule.empty())
				curSubmodule.append(L"\n");
			curSubmodule.append(lines[i]);
		}
	}

	if (!curSubmodule.empty()) {
		submodules.push_back({ curNum, curSubmodule });
	}

	return submodules;
}

std::vector<Question> SplitQuestions(const std::wstring& text, std::wregex& reg) {
	std::vector<Question> questions;

	std::wsmatch match;
	std::wstring wide_text = text;

	std::vector<std::wstring> lines;
	std::wistringstream iss(wide_text);
	std::wstring line;
	while (std::getline(iss, line)) {
		lines.push_back(line);
	}

	std::wstring current_question;
	int current_number = 0;
	bool in_question = false;

	for (size_t i = 0; i < lines.size(); ++i) {
		if (std::regex_search(lines[i], match, reg)) {
			if (in_question)
			{
				if (!current_question.empty()) {
					questions.push_back({ current_number, current_question });
					current_question.clear();
				}
			}
			current_number = stoi(match[1].str());
			in_question = true;
			current_question = lines[i];
		}
		else if (in_question)
		{
			if (!current_question.empty())
				current_question.append(L"\n");
			current_question.append(lines[i]);
		}
	}

	if (!current_question.empty()) {
		questions.push_back({ current_number, current_question });
	}

	return questions;
}

FormatQuestion handleData(const std::wstring& text)
{
	//std::wregex pattern1(LR"(^(\d+)\.\s*((?:.|\r|\n)*?)(?=A[\.\)��]))");
	std::wregex pattern2(LR"(([ABCD])[\.\)����]\s*((?:[^\n\rABCD]+(?:\r?\n)?)+))");
	std::wsmatch match1;

	std::wstring line;
	std::vector<std::wstring> lines;
	std::wistringstream iss(text);
	while (std::getline(iss, line)) {
		lines.push_back(line);
	}

	std::wstring tmp;
	for (size_t i = 0; i < lines.size(); ++i) {
		if (!std::regex_search(lines[i], match1, pattern2)) {
			if (tmp.empty())
				tmp = lines[i];
			else
			{
				tmp += L"\n";
				tmp += lines[i];
			}
		}
		else
		{
			break;
		}
	}

	FormatQuestion q;
	q.number = -1;
	q.question = tmp;
	q.answerNum = -1;
	q.answers = {};
	q.imgPaths = {};

	auto begin = std::wsregex_iterator(text.begin(), text.end(), pattern2);
	auto end = std::wsregex_iterator();

	for (auto it = begin; it != end; ++it) {
		std::wsmatch match = *it;
		q.answers.push_back({ match[2],"" });
	}

	return q;
}

void test(const std::string& path)
{
	auto tt = getTotalText(path);
	std::ofstream("D:\\Aliyat\\AliyatRenderer\\out\\build\\x64-Debug\\debug.txt") << tt;
	std::wregex moduleRegex(LR"((���Ϸ���)|(�ж�����)|(�������)|(������ϵ)|(��ʶ�ж�))");
	std::wregex subsection_pattern(LR"((^[һ�����������߰˾�ʮ]+��)|(��[һ�����������߰˾�ʮ]+��))");
	std::wregex question_pattern(LR"(^(\d+)[\.��]\s*((?:.*(?:\n(?!\d+[\.��]).*)*)+))");

	auto modules = SplitModules(utf8_to_wstring(tt), moduleRegex);

	for (const auto& m : modules) {
		std::wsmatch match;
		std::wregex wregex_1(LR"(���Ϸ���)");
		std::wregex wregex_2(LR"(�ж�����)");
		std::wregex wregex_3(LR"(�������)");
		std::wregex wregex_4(LR"(������ϵ)");
		std::wregex wregex_5(LR"(��ʶ�ж�)");


		if (std::regex_search(m.content, match, wregex_1))
		{
			auto submodules = SplitSubModules(m.content, subsection_pattern);
			for (const auto& sm : submodules)
			{
				/*std::cout << "���Ϸ��� " << sm.number << ":\n"
					<< utf8_to_gbk(wstring_to_utf8(sm.content)) << "\n\n";*/
			}
		}
		else if (std::regex_search(m.content, match, wregex_2))
		{
			auto submodules = SplitQuestions(m.content, question_pattern);
			for (const auto& sm : submodules)
			{
				auto q = handleData(sm.content);
				q.number = sm.number;
				std::cout << "�ж����� " << q.number << ":\n"
					<< utf8_to_gbk(wstring_to_utf8(q.question)) << "\n"
					<< "\tѡ� \n";
				for (int i = 0; i < q.answers.size(); i++)
					std::cout << utf8_to_gbk(wstring_to_utf8(q.answers[i].text)) << "\n";
				std::cout << std::endl;
			}
		}
		else if (std::regex_search(m.content, match, wregex_3))
		{
			auto submodules = SplitQuestions(m.content, question_pattern);
			for (const auto& sm : submodules)
			{
				auto q = handleData(sm.content);
				q.number = sm.number;
				std::cout << "������� " << q.number << ":\n"
					<< utf8_to_gbk(wstring_to_utf8(q.question)) << "\n"
					<< "\tѡ� \n";
				for (int i = 0; i < q.answers.size(); i++)
					std::cout << utf8_to_gbk(wstring_to_utf8(q.answers[i].text)) << "\n";
				std::cout << std::endl;
			}
		}
		else if (std::regex_search(m.content, match, wregex_4))
		{
			auto submodules = SplitQuestions(m.content, question_pattern);
			for (const auto& sm : submodules)
			{
				auto q = handleData(sm.content);
				q.number = sm.number;
				std::cout << "������ϵ " << q.number << ":\n"
					<< utf8_to_gbk(wstring_to_utf8(q.question)) << "\n"
					<< "\tѡ� \n";
				for (int i = 0; i < q.answers.size(); i++)
					std::cout << utf8_to_gbk(wstring_to_utf8(q.answers[i].text)) << "\n";
				std::cout << std::endl;
			}
		}
		else if (std::regex_search(m.content, match, wregex_5))
		{
			auto submodules = SplitQuestions(m.content, question_pattern);
			for (const auto& sm : submodules)
			{
				auto q = handleData(sm.content);
				q.number = sm.number;
				std::cout << "��ʶ�ж� " << q.number << ":\n"
					<< utf8_to_gbk(wstring_to_utf8(q.question)) << "\n"
					<< "\tѡ� \n";
				for (int i = 0; i < q.answers.size(); i++)
					std::cout << utf8_to_gbk(wstring_to_utf8(q.answers[i].text)) << "\n";
				std::cout << std::endl;
			}
		}
		else
		{

		}
	}
}

std::vector<std::pair<poppler::image, poppler::rectf>> extract_page_images(
	const poppler::page* page,
	const std::string& output_dir,
	int page_num
) {
	std::vector<std::pair<poppler::image, poppler::rectf>> result;

	// ����1��ͨ����Ⱦ����ȡͼƬ���Ƽ���
	poppler::page_renderer renderer;
	renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
	renderer.set_render_hint(poppler::page_renderer::render_hint::text_antialiasing, true);

	poppler::image img = renderer.render_page(page);
	if (img.is_valid()) {
		std::string path = output_dir + "/page_" + std::to_string(page_num) + ".png";
		if (img.save(path, "png")) {
			// ȫҳͼƬ��rect������ҳ��
			result.emplace_back(img, poppler::rectf(0, 0, page->page_rect().width(), page->page_rect().height()));
		}
	}

	auto text_list = page->text_list();
	for (const auto& text_box : text_list) {
		if (text_box.text().empty()) { // ������ͼƬ����
			poppler::rectf rect = text_box.bbox();
			// ��ȡ�ض�����
			poppler::image sub_img = renderer.render_page(page, rect.x(), rect.y(),
				rect.width(), rect.height());
			if (sub_img.is_valid()) {
				std::string img_path = output_dir + "/img_" + std::to_string(page_num) +
					"_" + std::to_string(rect.x()) + "_" +
					std::to_string(rect.y()) + ".png";
				if (sub_img.save(img_path, "png")) {
					result.emplace_back(sub_img, rect);
				}
			}
		}
	}
	return result;
}

struct ExamQuestion {
	int number;
	std::string content;
	poppler::rectf bbox;
	std::vector<std::string> image_paths;
	std::vector<poppler::rectf> image_rects;
};

std::vector<ExamQuestion> process_exam_page(
	const poppler::page* page,
	const std::string& output_dir,
	int page_num
) {
	std::vector<ExamQuestion> questions;

	// 1. ��ȡҳ��ͼƬ
	auto page_images = extract_page_images(page, output_dir, page_num);

	// 2. ��ȡ�ı�����
	auto text_layout = page->text_list();
	std::regex q_regex(R"((\d+)\.\s+([^\n]+))");

	// 3. ������Ŀ��ͼƬ
	for (const auto& text_box : text_layout) {
		auto texta = text_box.text().to_utf8();
		std::string utf8_text(texta.data(), texta.size());
		std::string text = utf8_text;
		std::smatch match;

		if (std::regex_search(text, match, q_regex)) {
			ExamQuestion q;
			q.number = std::stoi(match[1]);
			q.content = text;
			q.bbox = text_box.bbox();

			// ���Ҹ�����ͼƬ��Y�����<50�㣩
			for (const auto& [img, rect] : page_images) {
				if (std::abs(rect.y() - q.bbox.y()) < 50.0) {
					std::string path = output_dir + "/q" + std::to_string(q.number) +
						"_" + std::to_string(rect.x()) + ".png";
					if (img.save(path, "png")) {
						q.image_paths.push_back(path);
						q.image_rects.push_back(rect);
					}
				}
			}
			questions.push_back(q);
		}
	}

	return questions;
}



int main()
{
	//std::wstring ws(LR"(51.(1)��������֦��Ҷï      (2)����ʩ����·     (3)������ƽ���ƶ�300 ��
	//	(4)�о��ƶ��ƶ�����(5)�����ȷ���ѿ
	//	A.5��2��3��4��1    B.1��2��4��3��5    C.1��2��3��4��5   D.2��1��4��3��5
	//	- 7 -
	//	52.(1)Ӱ��ԭ�Ϳ���(2)����ʱ������(3)�뺣�ں�ˮ����(4)��ˮ�˷�����
	//	(5)���ﱻ�ȼ���
	//	A.3��1��5��4��2        B.2��4��3��5��1   C.5��3��1��4��2        D.4��2��3��1��5
	//	)");
	//std::wregex pattern1(LR"((\d+)\.\s*((?:.|\n)*?)(?=[A][\.\)��]))");
	//std::wregex pattern2(LR"(([ABCD])[\.\)]\s*([^ABCD]+))");
	//std::wsmatch match1;
	//if (std::regex_search(ws, match1, pattern1))
	//	std::cout << "hello";
	//getchar();

	//SetConsoleOutputCP(CP_UTF8);
	//_setmode(_fileno(stdout), _O_U8TEXT);
	auto str = OpenFileExplorerAndGetPath();
	if (!str.empty())
		test(wstring_to_utf8(str));



	return 0;
}