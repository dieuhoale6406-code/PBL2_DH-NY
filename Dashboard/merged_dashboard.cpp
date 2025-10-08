#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <tuple>
#include <ctime>
using namespace std;

// ====== Data models ======
struct Warehouse { string id, name, manager; };   // Kho
struct Receipt   { string id, whId, date; int qty{}; };  // Phiếu
struct History   { string time, user, action; };  // Lịch sử
struct WhMeta    { string id, status, createdAt, createdBy; }; // Meta kho
struct Account   { string user, pass, role; };    // Tài khoản (role: admin/staff)
struct Profile {string user, email, phone, createdAt;};
struct PermissionUIState {
    string search;
    int selStaffIndex = -1;
    vector<string> staffList; // usernames role=staff
    // cache checkbox bounds cho click
    vector<pair<sf::FloatRect,string>> whChecks; // rect, warehouseId
    sf::FloatRect btnSave{}, btnBack{};
};

// ====== [ADD] Console ======
struct ConsoleState {
    string input;
    vector<string> log; // hiển thị kết quả
    sf::FloatRect btnBack{}, btnEnter{};
};

// ====== Hồ sơ & phân quyền ======

static vector<Profile> gProfiles; 
static map<string, set<string>> gPermissions; 
static string gLoggedUser;
static PermissionUIState gPermUI;
static ConsoleState gConsole;
static int calcTon(const vector<Receipt>& inV, const vector<Receipt>& outV, const string& wh){
    long long in=0,out=0;
    for(const auto& r: inV) if(r.whId==wh) in += r.qty;
    for(const auto& r: outV) if(r.whId==wh) out += r.qty;
    long long ton = in - out; if(ton<0) ton=0;
    return (int)ton;
}

// ====== Utils & forward declarations ======
static string trim(string s); 
static string toLower(string s){ transform(s.begin(),s.end(),s.begin(),::tolower); return s; }
static bool icontains(const string& a0, const string& b0){
    string a=toLower(a0), b=toLower(b0); return a.find(b)!=string::npos;
}
static string fmtDate(const string& iso){
    if(iso.size()==10 && (iso[4]=='-'||iso[4]=='/') && (iso[7]=='-'||iso[7]=='/'))
        return iso.substr(8,2)+"/"+iso.substr(5,2)+"/"+iso.substr(0,4);
    return iso;
}

// ====== CSV loaders ======
static vector<Warehouse> loadWarehouses(const string& path) {
    vector<Warehouse> v; ifstream f(path); if(!f) return v;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        Warehouse w;
        getline(ss,w.id,','); getline(ss,w.name,','); getline(ss,w.manager,',');
        w.id=trim(w.id); w.name=trim(w.name); w.manager=trim(w.manager);
        if(!w.id.empty()) v.push_back(w);
    } return v;
}
static vector<Receipt> loadReceipts(const string& path){
    vector<Receipt> v; ifstream f(path); if(!f) return v;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line); Receipt r; string qty;
        getline(ss,r.id,','); getline(ss,r.whId,','); getline(ss,r.date,','); getline(ss,qty,',');
        r.id=trim(r.id); r.whId=trim(r.whId); r.date=trim(r.date);
        r.qty = qty.empty()?0:stoi(qty);
        if(!r.id.empty()) v.push_back(r);
    } return v;
}
static vector<History> loadHistory(const string& path) {
    vector<History> v; ifstream f(path); if(!f) return v;
    string line;
    while(getline(f,line)) {
        if(line.empty()) continue;
        stringstream ss(line); History h;
        getline(ss,h.time,','); getline(ss,h.user,','); getline(ss,h.action,',');
        h.time=trim(h.time); h.user=trim(h.user); h.action=trim(h.action);
        if(!h.time.empty()) v.push_back(h);
    } return v;
}
static vector<Account> loadAccounts(const string& path){
    vector<Account> v; ifstream f(path); if(!f) return v;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line); Account a;
        getline(ss,a.user,','); getline(ss,a.pass,','); getline(ss,a.role,',');
        a.user=trim(a.user); a.pass=trim(a.pass); a.role=trim(a.role);
        transform(a.role.begin(), a.role.end(), a.role.begin(), ::tolower);
        if(!a.user.empty()) v.push_back(a);
    } return v;
}
static vector<WhMeta> loadWhMeta(const string& path){
    vector<WhMeta> v; ifstream f(path); if(!f) return v;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line); WhMeta m;
        getline(ss,m.id,','); getline(ss,m.status,',');
        getline(ss,m.createdAt,','); getline(ss,m.createdBy,',');
        m.id=trim(m.id); m.status=trim(m.status);
        transform(m.status.begin(), m.status.end(), m.status.begin(), ::tolower);
        m.createdAt=trim(m.createdAt); m.createdBy=trim(m.createdBy);
        if(!m.id.empty()) v.push_back(m);
    } return v;
}
static vector<Profile> loadProfiles(const string& path) {
    vector<Profile> v; ifstream f(path); if(!f) return v;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line); Profile p;
        getline(ss,p.user,','); getline(ss,p.email,','); getline(ss,p.phone,','); getline(ss,p.createdAt,',');
        p.user=trim(p.user); p.email=trim(p.email); p.phone=trim(p.phone); p.createdAt=trim(p.createdAt);
        if(!p.user.empty()) v.push_back(p);
    } return v;
}
static map<string,set<string>> loadPermissions(const string& path){
    map<string,set<string>> mp; ifstream f(path); if(!f) return mp;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line); string u,w;
        getline(ss,u,','); getline(ss,w,',');
        u=trim(u); w=trim(w);
        if(!u.empty() && !w.empty()) mp[u].insert(w);
    } return mp;
}
static bool savePermissions(const string& path, const map<string,set<string>>& mp){
    ofstream out(path, ios::trunc);
    if(!out) return false;
    for(const auto& kv: mp){
        for(const auto& wid: kv.second)
            out << kv.first << "," << wid << "\n";
    }
    return true;
}

// ====== Misc ======
enum class Tab { Kho, Nhap, Xuat, Ton, LichSu };
enum class View { Dashboard, Profile, Permission, Console };
static View gView = View::Dashboard;
struct NavItem { string label; sf::FloatRect hit; };

static string trim(string s) {
    while(!s.empty() && (s.back()=='\r' || s.back()=='\n' || s.back()==' ')) s.pop_back();
    size_t i=0; while(i<s.size() && s[i]==' ') ++i; return s.substr(i);
}
static string todayISO() {
    time_t t = time(nullptr); tm *lt = localtime(&t);
    char buf[16]; snprintf(buf,sizeof(buf),"%04d-%02d-%02d",lt->tm_year+1900,lt->tm_mon+1,lt->tm_mday);
    return string(buf);
}
static void appendReceiptCSV(const string& path, const Receipt& r) {
    ofstream out(path, ios::app);
    if(out) out << r.id << "," << r.whId << "," << r.date << "," << r.qty << "\n";
}

// ==== Warehouse Detail View state ====
enum class WhView { List, Detail };
static WhView whView = WhView::List;
static string selWhId, selWhName, selWhManager;
static vector<tuple<sf::FloatRect,string,string,string>> whRowHitsFull;
static sf::FloatRect gHitBack;
// --- Detail extras from Dashboard build ---
static sf::FloatRect gHitDelete, gHitDelYes, gHitDelNo;
static bool showDelConfirm = false;
static float detailScroll = 0.f;
static float detailMaxScroll = 0.f;

// Delete helper (remove warehouse + meta)
static void deleteWarehouseById(const string& id,
                                vector<Warehouse>& warehouses,
                                vector<WhMeta>& whMeta)
{
    warehouses.erase(remove_if(warehouses.begin(), warehouses.end(),
                               [&](const Warehouse& w){ return w.id==id; }),
                     warehouses.end());
    whMeta.erase(remove_if(whMeta.begin(), whMeta.end(),
                           [&](const WhMeta& m){ return m.id==id; }),
                 whMeta.end());
}

// Demo chart flag
static const bool USE_DEMO_CHART = true;


void drawTable(sf::RenderWindow& window, sf::Font& font,
               sf::Vector2f pos, // pos: vị trí góc trên bên trái của bảng
               const vector<string>& headers, // headers: mảng tiêu đề cột
               const vector<vector<string>>& rows, // rows: mảng các hàng, mỗi hàng là mảng các cột
               float scrollOffset = 0.f) // scrollOffset: độ dịch chuyển dọc (do scroll)
{
    const float rowH = 35.f;
    float totalW = 0.f;
    vector<float> colW;
    for(size_t i=0;i<headers.size();++i){
        if(headers[i] == "Hanh dong") colW.push_back(400.f);
        else if(headers[i] == "Ten kho") colW.push_back(200.f);
        else colW.push_back(150.f);
        totalW += colW.back();
    }

    // Header
    sf::RectangleShape header(sf::Vector2f(totalW,rowH));
    header.setFillColor(sf::Color(200,200,200));
    header.setPosition(sf::Vector2f(pos.x, pos.y - scrollOffset));
    window.draw(header);

    float x = pos.x;
    for(size_t i=0;i<headers.size();++i) {
        sf::Text t(font, headers[i], 16);
        t.setFillColor(sf::Color::Black);
        t.setPosition(sf::Vector2f(x + 5.f, pos.y + 8.f - scrollOffset));
        window.draw(t);
        x += colW[i];
    }

    // Rows
    for(size_t r=0;r<rows.size();++r) {
        sf::RectangleShape row(sf::Vector2f(totalW, rowH));
        row.setFillColor(r%2 ? sf::Color(245,245,245) : sf::Color(255,255,255));
        row.setPosition(sf::Vector2f(pos.x, pos.y + (r+1)*rowH - scrollOffset));
        window.draw(row);

        float x2 = pos.x;
        for(size_t c=0;c<rows[r].size() && c<colW.size();++c){
            sf::Text t(font, rows[r][c], 15);
            t.setFillColor(sf::Color::Black);
            t.setPosition(sf::Vector2f(x2 + 5.f, pos.y + (r+1)*rowH + 8.f - scrollOffset));
            window.draw(t);
            x2 += colW[c];
        }
    }
}

void drawWarehouseDetail(sf::RenderWindow& window, sf::Font& font,
                         const string& whId, const string& whName, const string& whManager,
                         const sf::RectangleShape& navBar, const sf::CircleShape& bubble,
                         const int navCount, const string* labels)
{
    window.clear(sf::Color(245,230,140));

        // ==== VIEW BEGIN: thiết lập view để cuộn nội dung (nav giữ cố định) ====
    auto baseView = window.getView();
    sf::View scView = baseView;

    // vùng nhìn thấy (bỏ phần navBar ở dưới + thêm margin)
    float usableH = baseView.getSize().y - (navBar.getSize().y + 40.f);

    // tổng chiều cao content (tới cuối 2 bảng); layout hiện tại ~820px
    float contentH = 1000.f;
    detailMaxScroll = std::max(0.f, contentH - usableH);

    // clamp scroll
    if (detailScroll < 0.f) detailScroll = 0.f;
    if (detailScroll > detailMaxScroll) detailScroll = detailMaxScroll;

    // dịch view theo scroll
    sf::Vector2f c = baseView.getCenter(); // tâm gốc của view
    c.y += detailScroll;                   // cộng phần cuộn
    scView.setCenter(c);
    window.setView(scView);
    // ==== VIEW BEGIN - END ====


    // Title
    sf::Text title(font, ("Kho - " + whName), 26);
    title.setFillColor(sf::Color(25,42,60));
    title.setPosition({30.f, 20.f});
    window.draw(title);

    // Back button
    sf::RectangleShape backBtn({120.f,34.f});
    backBtn.setFillColor(sf::Color(230,240,255));
    backBtn.setOutlineThickness(1.f);
    backBtn.setOutlineColor(sf::Color(160,170,200));
    backBtn.setPosition({30.f, 64.f});
    sf::Text backTxt(font, "<- Quay lai", 18);
    backTxt.setFillColor(sf::Color(30,80,160));
    backTxt.setPosition(backBtn.getPosition() + sf::Vector2f{12.f,6.f});
    window.draw(backBtn); window.draw(backTxt);
    gHitBack = backBtn.getGlobalBounds(); // expose cho event loop
    
    // [ADD] Nút "🗑️ Xoa kho" (góc phải trên)
    const float delW = 140.f, delH = 34.f;
    sf::RectangleShape delBtn({delW, delH});
    delBtn.setFillColor(sf::Color(220, 60, 60));
    delBtn.setOutlineThickness(1.f);
    delBtn.setOutlineColor(sf::Color(200,40,40));
    delBtn.setPosition({
        static_cast<float>(window.getSize().x) - 30.f - delW, // canh phải
        20.f
    });

    // text có icon ở trước
    sf::Text delTxt(font, "Xoa kho", 18);
    delTxt.setFillColor(sf::Color::White);
    // canh giữa tương đối trong button (SFML 3 cần Vector2f)
    auto bb = delTxt.getLocalBounds();
    delTxt.setPosition(sf::Vector2f(
        delBtn.getPosition().x + (delW - bb.size.x) * 0.5f,
        delBtn.getPosition().y + (delH - bb.size.y) * 0.5f - 2.f
    ));


    window.draw(delBtn); window.draw(delTxt);
    gHitDelete = delBtn.getGlobalBounds();


    // Thẻ thông tin (trái)
    sf::RectangleShape meta({700.f, 110.f});
    meta.setFillColor(sf::Color(250,250,250));
    meta.setOutlineThickness(1.f); meta.setOutlineColor(sf::Color(200,200,200));
    meta.setPosition({30.f, 112.f});
    window.draw(meta);

    sf::Text tInfo(font,
        "Nguoi tao: FW.SP.MN Ngay tao: --/--/---- Quan ly: " + whManager,16);
    tInfo.setFillColor(sf::Color(60,60,60));
    tInfo.setPosition(meta.getPosition() + sf::Vector2f{16.f,14.f});
    window.draw(tInfo);

    // KPI (phải) — bố cục có khoảng cách an toàn
    auto kpiCard = [&](sf::Vector2f pos, const string& label, const string& val){
        sf::RectangleShape box({300.f, 52.f});
        box.setFillColor(sf::Color(250,250,250));
        box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(200,200,200));
        box.setPosition(pos); window.draw(box);
        sf::Text l(font, label, 16); l.setFillColor(sf::Color(90,90,90));
        l.setPosition(pos + sf::Vector2f{12.f, 8.f}); window.draw(l);
        sf::Text v(font, val, 20); v.setFillColor(sf::Color(25,42,60));
        v.setPosition(pos + sf::Vector2f{12.f, 26.f}); window.draw(v);
    };

    // toạ độ & khoảng cách
    const float panelRightX = 760.f;
    const float kpiH = 52.f;
    const float kpiGap = 14.f;
    const float kpiY1 = 112.f;
    const float kpiY2 = kpiY1 + kpiH + kpiGap;         // 112 + 52 + 14 = 178
    const float kpiY3 = kpiY2 + kpiH + kpiGap;         // 178 + 52 + 14 = 244

    kpiCard({panelRightX, kpiY1}, "So phieu nhap", "0");
    kpiCard({panelRightX, kpiY2}, "So phieu xuat", "0");
    kpiCard({panelRightX, kpiY3}, "Hang hoa ton kho", "0");

    // Chart — đặt dưới phần cao nhất (meta hoặc KPI) + thêm đệm
    float metaBottom = 112.f + 110.f;                  // y meta + cao meta
    float kpiBottom  = kpiY3 + kpiH;                   // đáy ô KPI thứ 3
    float topBlock   = std::max(metaBottom, kpiBottom);
    float chartY     = topBlock + 24.f;                // đệm 24px

    sf::Text chartTitle(font, "Tinh hinh nhap, xuat kho 12 thang gan day", 18);
    chartTitle.setFillColor(sf::Color(25,42,60));
    chartTitle.setPosition({30.f, chartY - 28.f});     // tiêu đề nằm trên chart
    window.draw(chartTitle);

    // chiều rộng chart theo cửa sổ để tránh tràn
    float chartW = static_cast<float>(window.getSize().x) - 60.f; // margin 30 mỗi bên
    sf::RectangleShape chart({chartW, 280.f});
    chart.setFillColor(sf::Color(255,255,255));
    chart.setOutlineThickness(1.f); chart.setOutlineColor(sf::Color(200,200,200));
    chart.setPosition({30.f, chartY});
    window.draw(chart);


    // ==== CHART: 12 tháng, mỗi tháng 2 cột (Nhập/Xuất) + lưới + nhãn ====

    // Khu vực vẽ
    float cx = chart.getPosition().x;
    float cy = chart.getPosition().y;
    float cw = chart.getSize().x;
    float ch = chart.getSize().y;

    // Padding bên trong chart
    const float padL = 44.f;   // để chỗ cho nhãn trục Y
    const float padR = 16.f;
    const float padT = 16.f;
    const float padB = 38.f;   // để chỗ nhãn tháng dưới trục X

    // Vùng đồ thị thực sự
    float gx = cx + padL;
    float gy = cy + padT;
    float gw = cw - (padL + padR);
    float gh = ch - (padT + padB);

    // Lưới ngang + nhãn trục Y (0 → 20M)
    const int Y_STEPS = 4;              // 0, 5M, 10M, 15M, 20M (5 vạch)
    const float Y_MAX = 20000000.f;     // 20M
    sf::Color gridCol(235,235,235);

    for(int i=0; i<=Y_STEPS; ++i){
        float t = (float)i / (float)Y_STEPS;
        float y = gy + gh - t * gh;

        sf::RectangleShape grid({gw, 1.f});
        grid.setFillColor(gridCol);
        grid.setPosition({gx, y});
        window.draw(grid);

        // Nhãn số bên trái
        int labelVal = (int)(Y_MAX * t);
        string label;
        if (labelVal==0) label = "0";
        else {
            // hiển thị theo “5M, 10M, …”
            label = to_string(labelVal / 1000000) + string("M");
        }
        sf::Text yt(font, label, 13);
        yt.setFillColor(sf::Color(90,90,90));
        auto b = yt.getLocalBounds();
        yt.setPosition({gx - 8.f - b.size.x, y - b.size.y/2.f - 2.f});
        window.draw(yt);
    }

    // Nhãn tháng: 12 mốc gần nhất (tháng/năm)
    vector<string> months; months.reserve(12);
    {
        std::time_t tt = std::time(nullptr);
        std::tm* tm = std::localtime(&tt);
        int mNow = tm ? (tm->tm_mon + 1) : 1;
        int yNow = tm ? (tm->tm_year + 1900) : 2025;
        int mStart = mNow - 11, yStart = yNow;
        while(mStart <= 0){ mStart += 12; --yStart; }
        int mm = mStart, yy = yStart;
        for(int k=0;k<12;k++){
            months.push_back(to_string(mm) + "/" + to_string(yy));
            ++mm; if(mm>12){ mm=1; ++yy; }
        }
    }

    // --- DỮ LIỆU BIỂU ĐỒ ---
    vector<float> vIn(12, 0.f), vOut(12, 0.f);

    if (USE_DEMO_CHART) {
        // Demo cố định (đẹp, tăng dần về các tháng gần đây)
        vIn  = { 1'000'000.f,  2'000'000.f,  3'000'000.f,  4'000'000.f,
                5'000'000.f,  6'500'000.f,  8'000'000.f, 10'000'000.f,
                12'000'000.f, 14'000'000.f, 17'000'000.f, 19'000'000.f };

        vOut = {   800'000.f,  1'500'000.f,  2'400'000.f,  3'500'000.f,
                4'200'000.f,  5'600'000.f,  7'200'000.f,  8'800'000.f,
                10'500'000.f, 12'800'000.f, 15'500'000.f, 16'000'000.f };
    } else {
        // (để trống tạm — sau này bạn gom dữ liệu thật ở đây)
        // vIn[k] / vOut[k] = tổng mỗi tháng theo months[]
        bool allZero = true;
        for (int i=0;i<12;i++) if (vIn[i]>0 || vOut[i]>0) { allZero=false; break; }
        if (allZero) {
            // fallback demo
            vIn  = { 1'000'000.f,  2'000'000.f,  3'000'000.f,  4'000'000.f,
                    5'000'000.f,  6'500'000.f,  8'000'000.f, 10'000'000.f,
                    12'000'000.f, 14'000'000.f, 17'000'000.f, 19'000'000.f };
            vOut = {   800'000.f,  1'500'000.f,  2'400'000.f,  3'500'000.f,
                    4'200'000.f,  5'600'000.f,  7'200'000.f,  8'800'000.f,
                    10'500'000.f, 12'800'000.f, 15'500'000.f, 16'000'000.f };
        }
    }
                    

    // --- cấu hình độ rộng cột mảnh và bám giữa tick ---
    float monthSlot = gw / 12.f;
    float barW      = std::min(12.f, monthSlot * 0.30f);   // mỗi cột ~10–12px
    float barGap    = std::max(3.f,  barW * 0.40f);        // khoảng cách giữa 2 cột

    // --- vẽ 12 tháng ---
    for (int k = 0; k < 12; ++k)
    {
        // tâm của tháng k (trùng vạch tick trên trục X)
        float xMid = gx + (k + 0.5f) * monthSlot;

        // Nhãn tháng dưới tick
        sf::Text mt(font, months[k], 13);
        mt.setFillColor(sf::Color(90,90,90));
        auto mb = mt.getLocalBounds();
        mt.setPosition({ xMid - mb.size.x/2.f, cy + ch - mb.size.y - 6.f });
        window.draw(mt);

        // Chuyển giá trị -> chiều cao cột (clamp về [0, Y_MAX])
        float vInClamped  = std::min<float>(vIn[k],  Y_MAX);
        float vOutClamped = std::min<float>(vOut[k], Y_MAX);
        float hIn  = (Y_MAX <= 0.f) ? 0.f : (vInClamped  / Y_MAX) * gh;
        float hOut = (Y_MAX <= 0.f) ? 0.f : (vOutClamped / Y_MAX) * gh;

        // Nhóm 2 cột ôm quanh tick
        float groupW = 2.f * barW + barGap;
        float leftX  = xMid - groupW * 0.5f;

        // Cột “Nhập” (trái)
        sf::RectangleShape rIn({ barW, hIn });
        rIn.setFillColor(sf::Color(200,220,255));
        rIn.setPosition({ leftX, gy + gh - hIn });
        window.draw(rIn);

        // Cột “Xuất” (phải)
        sf::RectangleShape rOut({ barW, hOut });
        rOut.setFillColor(sf::Color(160,160,160));
        rOut.setPosition({ leftX + barW + barGap, gy + gh - hOut });
        window.draw(rOut);
    }



    // Chú thích (legend) – đặt ngay dưới chart
    float legendY = cy + ch + 8.f;
    sf::RectangleShape sw1({16.f, 10.f}); sw1.setFillColor(sf::Color(200,220,255));
    sf::RectangleShape sw2({16.f, 10.f}); sw2.setFillColor(sf::Color(160,160,160));

    sf::Text lg1(font, "Phieu nhap", 14); lg1.setFillColor(sf::Color(25,42,60));
    sf::Text lg2(font, "Phieu xuat", 14); lg2.setFillColor(sf::Color(25,42,60));

    // canh giữa
    float totalLegendW = 16.f + 6.f + lg1.getLocalBounds().size.x + 24.f
                    + 16.f + 6.f + lg2.getLocalBounds().size.x;

    float legendX = cx + (cw - totalLegendW) * 0.5f;

    sw1.setPosition({legendX, legendY}); window.draw(sw1);
    lg1.setPosition({legendX + 16.f + 6.f, legendY - 3.f}); window.draw(lg1);

    float x2 = legendX + 16.f + 6.f + lg1.getLocalBounds().size.x + 24.f;
    sw2.setPosition({x2, legendY}); window.draw(sw2);
    lg2.setPosition({x2 + 16.f + 6.f, legendY - 3.f}); window.draw(lg2);


    // Hai bảng gần đây — đặt dưới chart, có khoảng đệm
    auto drawList = [&](sf::Vector2f pos, float width, const string& ttl) -> float {
        sf::Text tt(font, ttl, 18); tt.setFillColor(sf::Color(25,42,60));
        tt.setPosition(pos); window.draw(tt);

        const float rowH = 34.f;
        const float boxH = 220.f;

        sf::RectangleShape box({width, boxH});
        box.setFillColor(sf::Color(255,255,255));
        box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(200,200,200));
        box.setPosition(pos + sf::Vector2f{0.f, 30.f}); window.draw(box);

        sf::RectangleShape hd({box.getSize().x, rowH});
        hd.setFillColor(sf::Color(240,240,240));
        hd.setPosition(box.getPosition()); window.draw(hd);

        sf::Text h1(font, "#", 16), h2(font, "Ma phieu", 16),
                h3(font, "Nguoi ...", 16), h4(font, "Ngay", 16);
        h1.setFillColor(sf::Color::Black); h2.setFillColor(sf::Color::Black);
        h3.setFillColor(sf::Color::Black); h4.setFillColor(sf::Color::Black);
        float x = box.getPosition().x + 10.f;
        h1.setPosition({x,            box.getPosition().y + 7.f}); window.draw(h1);
        h2.setPosition({x+30.f,       box.getPosition().y + 7.f}); window.draw(h2);
        h3.setPosition({x+180.f,      box.getPosition().y + 7.f}); window.draw(h3);
        h4.setPosition({x+width-120.f,box.getPosition().y + 7.f}); window.draw(h4);

        sf::Text empty(font, "Chua co du lieu.", 16);
        empty.setFillColor(sf::Color(120,120,120));
        empty.setPosition({box.getPosition().x + 12.f, box.getPosition().y + rowH + 10.f});
        window.draw(empty);

        return box.getPosition().y + box.getSize().y; // trả về đáy để tính tiếp
    };

    // --- Vị trí phụ thuộc chart ---
    float gapTopLists = 28.f;             // đệm giữa chart và 2 bảng
    float gapBetween  = 24.f;             // đệm giữa 2 bảng trái-phải
    float listY       = chart.getPosition().y + chart.getSize().y + gapTopLists;
    float listW       = (chartW - gapBetween) * 0.5f;
    float leftX       = 30.f;
    float rightX      = leftX + listW + gapBetween;

    float bottomL = drawList({leftX,  listY}, listW, "Phieu nhap gan day");
    float bottomR = drawList({rightX, listY}, listW, "Phieu xuat gan day");

    // (tuỳ chọn) nếu vẫn thấy sát, có thể tăng contentH lên 1080.f hoặc tăng gapTopLists.

    // ==== VIEW RESET: trả view về mặc định để nav cố định ====
    window.setView(window.getDefaultView());

    // [ADD] Popup xác nhận xóa (Admin xóa thật)
    if (showDelConfirm) {
        // nền mờ
        sf::RectangleShape shade({static_cast<float>(window.getSize().x),
                                static_cast<float>(window.getSize().y)});
        shade.setFillColor(sf::Color(0,0,0,110));
        window.draw(shade);

        // hộp xác nhận
        sf::Vector2f boxSz{460.f, 190.f};
        sf::Vector2f boxPos{
            window.getSize().x * 0.5f - boxSz.x * 0.5f,
            window.getSize().y * 0.45f - boxSz.y * 0.5f
        };
        sf::RectangleShape box(boxSz);
        box.setFillColor(sf::Color(255,255,255));
        box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(200,200,200));
        box.setPosition(boxPos);
        window.draw(box);

        // tiêu đề + nội dung
        sf::Text titleC(font, "Xac nhan xoa kho", 18);
        titleC.setFillColor(sf::Color(25,42,60));
        titleC.setPosition(boxPos + sf::Vector2f{18.f, 16.f});
        window.draw(titleC);

        sf::Text msg(font, "Ban co chac muon xoa kho \"" + whName + "\"?", 16);
        msg.setFillColor(sf::Color(60,60,60));
        msg.setPosition(boxPos + sf::Vector2f{18.f, 54.f});
        window.draw(msg);

        // nút Hủy
        sf::RectangleShape btnNo({130.f, 40.f});
        btnNo.setFillColor(sf::Color(235,235,235));
        btnNo.setOutlineThickness(1.f); btnNo.setOutlineColor(sf::Color(180,180,180));
        btnNo.setPosition(boxPos + sf::Vector2f{boxSz.x - 290.f, boxSz.y - 56.f});
        sf::Text noTxt(font, "Huy", 18); noTxt.setFillColor(sf::Color::Black);
        auto nb = noTxt.getLocalBounds();
        noTxt.setPosition(sf::Vector2f(
            btnNo.getPosition().x + (130.f - nb.size.x)/2.f,
            btnNo.getPosition().y + (40.f - nb.size.y)/2.f - 3.f
        ));

        window.draw(btnNo); window.draw(noTxt);

        // nút Xóa (đỏ)
        sf::RectangleShape btnYes({130.f, 40.f});
        btnYes.setFillColor(sf::Color(220,60,60));
        btnYes.setOutlineThickness(1.f); btnYes.setOutlineColor(sf::Color(200,40,40));
        btnYes.setPosition(boxPos + sf::Vector2f{boxSz.x - 145.f, boxSz.y - 56.f});
        sf::Text yesTxt(font, "Xoa", 18); yesTxt.setFillColor(sf::Color::White);
        auto yb = yesTxt.getLocalBounds();
        yesTxt.setPosition(sf::Vector2f(
            btnYes.getPosition().x + (130.f - yb.size.x)/2.f,
            btnYes.getPosition().y + (40.f - yb.size.y)/2.f - 3.f
        ));

        window.draw(btnYes); window.draw(yesTxt);

        // expose vùng click
        gHitDelNo  = btnNo.getGlobalBounds();
        gHitDelYes = btnYes.getGlobalBounds();
    }


    // Vẽ nav bar & bubble + label (để đồng bộ layout)
    window.draw(navBar); window.draw(bubble);
    for(int i=0;i<navCount;i++){
        float cellW = navBar.getSize().x / navCount;
        float cx = navBar.getPosition().x + i*cellW + cellW*0.5f;
        float cy = navBar.getPosition().y + navBar.getSize().y*0.55f;
        sf::CircleShape icon(16.f);
        icon.setOrigin({16.f,16.f});
        icon.setFillColor(sf::Color::Transparent);
        icon.setOutlineThickness(2.5f);
        icon.setOutlineColor(sf::Color(220,220,220));
        icon.setPosition({cx,cy});
        window.draw(icon);

        sf::Text t(font, labels[i], 16);
        t.setFillColor(sf::Color(210,210,210));
        auto b = t.getLocalBounds();
        t.setPosition({cx - b.size.x/2.f, cy + 18.f});
        window.draw(t);
    }
}

    sf::RectangleShape chart({1030.f, 280.f});
    chart.setFillColor(sf::Color(255,255,255));
    chart.setOutlineThickness(1.f); chart.setOutlineColor(sf::Color(200,200,200));
    chart.setPosition({30.f, 270.f});
    window.draw(chart);

    // Skeleton cột (placeholder)
    for(int i=0;i<3;i++){
        sf::RectangleShape barIn({24.f, (float)(80 + i*10)});
        barIn.setFillColor(sf::Color(210,230,255));
        barIn.setPosition({chart.getPosition().x + 940.f + i*30.f, chart.getPosition().y + chart.getSize().y - barIn.getSize().y - 16.f});
        window.draw(barIn);

        sf::RectangleShape barOut({24.f, (float)(60 + i*10)});
        barOut.setFillColor(sf::Color(200,200,200));
        barOut.setPosition({barIn.getPosition().x + 26.f, chart.getPosition().y + chart.getSize().y - barOut.getSize().y - 16.f});
        window.draw(barOut);
    }

    // Hai bảng gần đây
    auto drawList = [&](sf::Vector2f pos, const string& ttl){
        sf::Text tt(font, ttl, 18); tt.setFillColor(sf::Color(25,42,60));
        tt.setPosition(pos); window.draw(tt);

        sf::RectangleShape box({500.f, 220.f});
        box.setFillColor(sf::Color(255,255,255));
        box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(200,200,200));
        box.setPosition(pos + sf::Vector2f{0.f, 30.f}); window.draw(box);

        const float rowH = 34.f;
        sf::RectangleShape hd({box.getSize().x, rowH});
        hd.setFillColor(sf::Color(240,240,240));
        hd.setPosition(box.getPosition()); window.draw(hd);

        sf::Text h1(font, "#", 16), h2(font, "Ma phieu", 16),
                 h3(font, "Nguoi ...", 16), h4(font, "Ngay", 16);
        h1.setFillColor(sf::Color::Black); h2.setFillColor(sf::Color::Black);
        h3.setFillColor(sf::Color::Black); h4.setFillColor(sf::Color::Black);
        float x = box.getPosition().x + 10.f;
        h1.setPosition({x, box.getPosition().y + 7.f}); window.draw(h1);
        h2.setPosition({x+30.f, box.getPosition().y + 7.f}); window.draw(h2);
        h3.setPosition({x+180.f, box.getPosition().y + 7.f}); window.draw(h3);
        h4.setPosition({x+380.f, box.getPosition().y + 7.f}); window.draw(h4);

        sf::Text empty(font, "Chua co du lieu.", 16);
        empty.setFillColor(sf::Color(120,120,120));
        empty.setPosition({box.getPosition().x + 12.f, box.getPosition().y + rowH + 10.f});
        window.draw(empty);
    };
    drawList({30.f, 570.f},  "Phieu nhap gan day");
    drawList({560.f, 570.f}, "Phieu xuat gan day");

    // Vẽ nav bar & bubble + label (để đồng bộ layout)
    window.draw(navBar); window.draw(bubble);
    for(int i=0;i<navCount;i++){
        float cellW = navBar.getSize().x / navCount;
        float cx = navBar.getPosition().x + i*cellW + cellW*0.5f;
        float cy = navBar.getPosition().y + navBar.getSize().y*0.55f;
        sf::CircleShape icon(16.f);
        icon.setOrigin({16.f,16.f});
        icon.setFillColor(sf::Color::Transparent);
        icon.setOutlineThickness(2.5f);
        icon.setOutlineColor(sf::Color(220,220,220));
        icon.setPosition({cx,cy});
        window.draw(icon);

        sf::Text t(font, labels[i], 16);
        t.setFillColor(sf::Color(210,210,210));
        auto b = t.getLocalBounds();
        t.setPosition({cx - b.size.x/2.f, cy + 18.f});
        window.draw(t);
    }
}

// ====== [ADD] Draw Profile ======
void drawProfile(sf::RenderWindow& window, sf::Font& font,
                 const vector<Profile>& profiles,
                 const map<string,set<string>>& perms,
                 const vector<Warehouse>& warehouses,
                 const string& username, bool isAdmin,
                 sf::RectangleShape& navBar, sf::CircleShape& bubble,
                 int navCount, const string* labels)
{
    window.clear(sf::Color(245,230,140));
    // khung
    sf::RectangleShape card({min(900.f, (float)window.getSize().x - 60.f), 420.f});
    card.setFillColor(sf::Color(255,255,255));
    card.setOutlineThickness(1.f); card.setOutlineColor(sf::Color(200,200,200));
    card.setPosition({30.f, 90.f});
    window.draw(card);

    // avatar
    sf::CircleShape av(36.f); av.setFillColor(sf::Color(230,240,255));
    av.setOutlineThickness(1.f); av.setOutlineColor(sf::Color(170,180,200));
    av.setPosition(card.getPosition()+sf::Vector2f{24.f,24.f});
    window.draw(av);

    sf::Text title(font, "Ho so ca nhan", 24);
    title.setFillColor(sf::Color(25,42,60));
    title.setPosition(card.getPosition()+sf::Vector2f{84.f,28.f});
    window.draw(title);

    auto findP = [&]()->Profile {
        for(const auto& p: profiles) if(p.user==username) return p;
        return Profile{username,"(chua co)","(chua co)","--/--/----"};
    }();
    auto label = [&](string k, string v, float y){
        sf::Text l(font, k, 16); l.setFillColor(sf::Color(90,90,90));
        l.setPosition(card.getPosition()+sf::Vector2f{24.f,y}); window.draw(l);
        sf::Text x(font, v, 18); x.setFillColor(sf::Color(25,42,60));
        x.setPosition(card.getPosition()+sf::Vector2f{180.f,y-2.f}); window.draw(x);
    };
    label("Username:", findP.user, 90.f);
    label("Role:", isAdmin? "admin":"staff", 122.f);
    label("Email:", findP.email, 154.f);
    label("So dien thoai:", findP.phone, 186.f);
    label("Ngay tao:", fmtDate(findP.createdAt), 218.f);

    // danh sách kho nếu là staff
    if(!isAdmin){
        sf::Text t(font,"Kho duoc phan quyen:",16); t.setFillColor(sf::Color(90,90,90));
        t.setPosition(card.getPosition()+sf::Vector2f{24.f,260.f}); window.draw(t);
        float y = 290.f; int count=0;
        auto it = perms.find(username);
        if(it==perms.end() || it->second.empty()){
            sf::Text e(font,"(Chua duoc phan quyen kho)",16);
            e.setFillColor(sf::Color(140,140,140));
            e.setPosition(card.getPosition()+sf::Vector2f{24.f,y});
            window.draw(e);
        }else{
            for(const string& wid: it->second){
                auto w = find_if(warehouses.begin(),warehouses.end(),
                                 [&](const Warehouse& x){return x.id==wid;});
                string line = wid + " - " + (w!=warehouses.end()? w->name : "(unknown)");
                sf::Text r(font,line,16); r.setFillColor(sf::Color(25,42,60));
                r.setPosition(card.getPosition()+sf::Vector2f{24.f,y});
                window.draw(r); y += 26.f; if(++count>10) break;
            }
        }
    }

    // nút quay lại Dashboard
    sf::RectangleShape back({130.f,34.f});
    back.setFillColor(sf::Color(230,240,255));
    back.setOutlineThickness(1.f); back.setOutlineColor(sf::Color(160,170,200));
    back.setPosition(card.getPosition()+sf::Vector2f{card.getSize().x-140.f, card.getSize().y-44.f});
    sf::Text bt(font,"← Quay lai",16); bt.setFillColor(sf::Color(30,80,160));
    bt.setPosition(back.getPosition()+sf::Vector2f{18.f,6.f});
    window.draw(back); window.draw(bt);

    // điều hướng chung (giữ bố cục)
    window.draw(navBar); window.draw(bubble);
    for(int i=0;i<navCount;i++){
        float cellW = navBar.getSize().x / navCount;
        float cx = navBar.getPosition().x + i*cellW + cellW*0.5f;
        float cy = navBar.getPosition().y + navBar.getSize().y*0.55f;
        sf::CircleShape icon(16.f); icon.setOrigin({16.f,16.f});
        icon.setFillColor(sf::Color::Transparent);
        icon.setOutlineThickness(2.5f); icon.setOutlineColor(sf::Color(220,220,220));
        icon.setPosition({cx,cy}); window.draw(icon);

        sf::Text t(font, labels[i], 16); t.setFillColor(sf::Color(210,210,210));
        auto b = t.getLocalBounds(); t.setPosition({cx - b.size.x/2.f, cy + 18.f});
        window.draw(t);
    }
}

void drawPermission(sf::RenderWindow& window, sf::Font& font,
                    const vector<Account>& accounts,
                    const vector<Warehouse>& warehouses,
                    map<string,set<string>>& perms,
                    PermissionUIState& UI)
{
    window.clear(sf::Color(245,230,140));

    // Lọc staff list theo search (khởi tạo nếu trống)
    UI.staffList.clear();
    for(const auto& a: accounts){
        if(a.role=="staff"){
            if(UI.search.empty() || icontains(a.user, UI.search))
                UI.staffList.push_back(a.user);
        }
    }

    // Layout 2 cột
    float pad = 30.f;
    float leftW = 380.f;
    float topY = 90.f;

    // Left: danh sách staff
    sf::RectangleShape left({leftW, (float)window.getSize().y - topY - 120.f});
    left.setFillColor(sf::Color(255,255,255));
    left.setOutlineThickness(1.f); left.setOutlineColor(sf::Color(200,200,200));
    left.setPosition({pad, topY});
    window.draw(left);

    sf::Text ttlL(font,"Chon Staff",20); ttlL.setFillColor(sf::Color(25,42,60));
    ttlL.setPosition(left.getPosition()+sf::Vector2f{14.f,10.f}); window.draw(ttlL);

    // ô search
    sf::RectangleShape sbox({leftW-28.f, 34.f});
    sbox.setFillColor(sf::Color(240,240,240));
    sbox.setOutlineThickness(1.f); sbox.setOutlineColor(sf::Color(160,160,160));
    sbox.setPosition(left.getPosition()+sf::Vector2f{14.f,44.f}); window.draw(sbox);

    sf::Text stext(font, UI.search.empty()? "Tim username..." : UI.search, 16);
    stext.setFillColor(UI.search.empty()? sf::Color(140,140,140): sf::Color::Black);
    stext.setPosition(sbox.getPosition()+sf::Vector2f{8.f,6.f}); window.draw(stext);

    // list
    float y = 90.f;
    for(size_t i=0;i<UI.staffList.size();++i){
        sf::RectangleShape row({leftW-28.f, 30.f});
        row.setPosition(left.getPosition()+sf::Vector2f{14.f, y});
        row.setFillColor((int)i==UI.selStaffIndex? sf::Color(230,240,255): sf::Color(248,248,248));
        window.draw(row);
        sf::Text t(font, UI.staffList[i], 16);
        t.setFillColor(sf::Color::Black);
        t.setPosition(row.getPosition()+sf::Vector2f{8.f,5.f});
        window.draw(t);
        y += 32.f; if(left.getPosition().y + y > left.getPosition().y + left.getSize().y - 50.f) break;
    }

    // Right: danh sách kho + checkbox
    float rightX = pad + leftW + 20.f;
    sf::RectangleShape right({(float)window.getSize().x - rightX - pad, (float)window.getSize().y - topY - 120.f});
    right.setFillColor(sf::Color(255,255,255));
    right.setOutlineThickness(1.f); right.setOutlineColor(sf::Color(200,200,200));
    right.setPosition({rightX, topY});
    window.draw(right);

    sf::Text ttlR(font,"Gan kho cho Staff",20); ttlR.setFillColor(sf::Color(25,42,60));
    ttlR.setPosition(right.getPosition()+sf::Vector2f{14.f,10.f}); window.draw(ttlR);

    // show user đang chọn
    string activeUser = (UI.selStaffIndex>=0 && UI.selStaffIndex<(int)UI.staffList.size())
                        ? UI.staffList[UI.selStaffIndex] : "(chua chon)";
    sf::Text u(font, string("Staff: ") + activeUser, 16); u.setFillColor(sf::Color(90,90,90));
    u.setPosition(right.getPosition()+sf::Vector2f{14.f,44.f}); window.draw(u);

    // danh sách kho: checkbox
    UI.whChecks.clear();
    float yy = 80.f; int col=0;
    float colW = (right.getSize().x - 28.f);
    // đơn cột cho gọn
    for(const auto& w: warehouses){
        sf::RectangleShape row({colW, 30.f});
        row.setPosition(right.getPosition()+sf::Vector2f{14.f, yy});
        row.setFillColor(sf::Color(248,248,248));
        window.draw(row);

        // checkbox
        sf::RectangleShape box({20.f,20.f});
        box.setPosition(row.getPosition()+sf::Vector2f{8.f,5.f});
        box.setFillColor(sf::Color(255,255,255));
        box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(160,160,160));
        window.draw(box);

        bool checked=false;
        if(activeUser!="(chua chon)"){
            auto it = perms.find(activeUser);
            if(it!=perms.end()) checked = it->second.count(w.id)>0;
        }
        if(checked){
            sf::RectangleShape tick({14.f,14.f});
            tick.setPosition(box.getPosition()+sf::Vector2f{3.f,3.f});
            tick.setFillColor(sf::Color(60,140,220)); window.draw(tick);
        }

        sf::Text label(font, w.id + " - " + w.name, 16);
        label.setFillColor(sf::Color::Black);
        label.setPosition(row.getPosition()+sf::Vector2f{36.f,5.f});
        window.draw(label);

        UI.whChecks.push_back({box.getGlobalBounds(), w.id});
        yy += 32.f;
        if(right.getPosition().y + yy > right.getPosition().y + right.getSize().y - 60.f) break;
    }

    // Buttons: Lưu + Quay lại
    UI.btnSave = sf::FloatRect(
        sf::Vector2f(
            right.getPosition().x + right.getSize().x - 140.f,
            right.getPosition().y + right.getSize().y - 44.f
        ),
        sf::Vector2f(130.f, 34.f)
    );

    UI.btnBack = sf::FloatRect(
        sf::Vector2f(
            left.getPosition().x + 10.f,
            left.getPosition().y + left.getSize().y - 44.f
        ),
        sf::Vector2f(130.f, 34.f)
);



    
   // --- đặt ngay trong drawPermission, chỗ bạn đang tạo 2 nút Save/Back ---
    auto drawBtn = [&](const sf::FloatRect& rect, const std::string& text, bool primary) {
        // Lấy toạ độ & kích thước từ rect
        const sf::Vector2f pos = rect.position;  // góc trên trái
        const sf::Vector2f sz  = rect.size;      // chiều rộng/cao

        // Vẽ nút
        sf::RectangleShape b(sz);
        b.setPosition(pos);
        b.setFillColor(primary ? sf::Color(60,140,220) : sf::Color(230,240,255));
        b.setOutlineThickness(1.f);
        b.setOutlineColor(primary ? sf::Color(60,140,220) : sf::Color(160,170,200));
        window.draw(b);

        // Vẽ chữ
        sf::Text t(font, text, 16);
        t.setFillColor(primary ? sf::Color::White : sf::Color(30,80,160));

        // Bounds của text (có offset left/top âm)
        const auto tb = t.getLocalBounds();
        const sf::Vector2f tbPos  = tb.position; // offset
        const sf::Vector2f tbSize = tb.size;     // kích thước thật

        // Căn giữa chữ vào trong rect
        t.setPosition({
            pos.x + (sz.x - tbSize.x) / 2.f - tbPos.x,
            pos.y + (sz.y - tbSize.y) / 2.f - tbPos.y
        });

        window.draw(t);
    };




    drawBtn(UI.btnBack, "← Quay lai", false);
    drawBtn(UI.btnSave, "Luu", true);
}

// ====== [ADD] Hàm thực thi lệnh console ======
static void execConsoleCmd(const string& cmdline,
                           vector<Receipt>& receiptsIn,
                           vector<Receipt>& receiptsOut,
                           map<string,int>& inCount,
                           map<string,int>& outCount)
{
    string s = cmdline;
    // tách token
    stringstream ss(s); string cmd; ss >> cmd;
    if(cmd.empty()) { gConsole.log.push_back("> (empty)"); return; }

    string cLow = toLower(cmd);
    if(cLow=="nhap"){
        string wh; int qty=0; ss >> wh >> qty;
        if(wh.empty() || qty<=0){ gConsole.log.push_back("> Sai cu phap: Nhap [MaKho] [SoLuong]"); return; }
        Receipt r; r.id = "IN" + to_string((int)receiptsIn.size()+1);
        r.whId = wh; r.date = todayISO(); r.qty = qty;
        receiptsIn.push_back(r); inCount[wh]++;
        appendReceiptCSV("receipts_in.csv", r);
        gConsole.log.push_back("> OK: Nhap " + to_string(qty) + " vao kho " + wh);
    }else if(cLow=="xuat"){
        string wh; int qty=0; ss >> wh >> qty;
        if(wh.empty() || qty<=0){ gConsole.log.push_back("> Sai cu phap: Xuat [MaKho] [SoLuong]"); return; }
        Receipt r; r.id = "OUT" + to_string((int)receiptsOut.size()+1);
        r.whId = wh; r.date = todayISO(); r.qty = qty;
        receiptsOut.push_back(r); outCount[wh]++;
        appendReceiptCSV("receipts_out.csv", r);
        gConsole.log.push_back("> OK: Xuat " + to_string(qty) + " tu kho " + wh);
    }else if(cLow=="ton"){
        string wh; ss >> wh;
        if(wh.empty()){ gConsole.log.push_back("> Sai cu phap: Ton [MaKho]"); return; }
        int t = calcTon(receiptsIn, receiptsOut, wh);
        gConsole.log.push_back("> Ton " + wh + " = " + to_string(t));
    }else{
        gConsole.log.push_back("> Lenh khong ho tro.");
    }
}


void drawConsole(sf::RenderWindow& window, sf::Font& font,
                 vector<Receipt>& receiptsIn,
                 vector<Receipt>& receiptsOut,
                 map<string,int>& inCount,
                 map<string,int>& outCount)
{
    window.clear(sf::Color(245,230,140));

    // khung console
    sf::RectangleShape box({min(900.f,(float)window.getSize().x-60.f), 420.f});
    box.setFillColor(sf::Color(255,255,255));
    box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(200,200,200));
    box.setPosition({30.f, 90.f}); window.draw(box);

    sf::Text ttl(font,"Console",22);
    ttl.setFillColor(sf::Color(25,42,60));
    ttl.setPosition(box.getPosition()+sf::Vector2f{16.f,10.f}); window.draw(ttl);

    // log view
    sf::RectangleShape logBox({box.getSize().x - 32.f, 300.f});
    logBox.setFillColor(sf::Color(248,248,248));
    logBox.setOutlineThickness(1.f); logBox.setOutlineColor(sf::Color(210,210,210));
    logBox.setPosition(box.getPosition()+sf::Vector2f{16.f,44.f});
    window.draw(logBox);

    float y = logBox.getPosition().y + 8.f;
    for(int i = max(0, (int)gConsole.log.size()-10); i < (int)gConsole.log.size(); ++i){
        sf::Text line(font, gConsole.log[i], 16);
        line.setFillColor(sf::Color(25,42,60));
        line.setPosition(logBox.getPosition()+sf::Vector2f{8.f, (float)8.f + (i - max(0,(int)gConsole.log.size()-10))*22.f});
        window.draw(line);
    }

    // input
    sf::RectangleShape ibox({box.getSize().x - 140.f, 34.f});
    ibox.setFillColor(sf::Color(240,240,240));
    ibox.setOutlineThickness(1.f); ibox.setOutlineColor(sf::Color(160,160,160));
    ibox.setPosition(box.getPosition()+sf::Vector2f{16.f, box.getSize().y - 50.f});
    window.draw(ibox);
    sf::Text it(font, gConsole.input.empty()? "Nhap lenh (Nhap/Xuat/Ton)..." : gConsole.input, 16);
    it.setFillColor(gConsole.input.empty()? sf::Color(140,140,140) : sf::Color::Black);
    it.setPosition(ibox.getPosition()+sf::Vector2f{8.f,6.f}); window.draw(it);

    // Enter + Back
    gConsole.btnEnter = sf::FloatRect( ibox.getPosition()+sf::Vector2f{ibox.getSize().x+10.f, 0.f} + sf::Vector2f{0.f,0.f},
                                       sf::Vector2f(52.f,34.f));
    gConsole.btnBack  = sf::FloatRect( box.getPosition()+sf::Vector2f{box.getSize().x-130.f, box.getSize().y-50.f},
                                       sf::Vector2f(120.f,34.f));
    auto drawBtn = [&](const sf::FloatRect& rect, const std::string& text, bool primary) {
        // Lấy toạ độ & kích thước từ rect
        const sf::Vector2f pos = rect.position;  // góc trên trái
        const sf::Vector2f sz  = rect.size;      // chiều rộng/cao

        // Vẽ nút
        sf::RectangleShape b(sz);
        b.setPosition(pos);
        b.setFillColor(primary ? sf::Color(60,140,220) : sf::Color(230,240,255));
        b.setOutlineThickness(1.f);
        b.setOutlineColor(primary ? sf::Color(60,140,220) : sf::Color(160,170,200));
        window.draw(b);

        // Vẽ chữ
        sf::Text t(font, text, 16); 
        t.setFillColor(primary ? sf::Color::White : sf::Color(30,80,160));

        // Bounds của text (có offset left/top âm)
        const auto tb = t.getLocalBounds();
        const sf::Vector2f tbPos  = tb.position; // offset
        const sf::Vector2f tbSize = tb.size;     // kích thước thật

        // Căn giữa chữ vào trong rect
        t.setPosition({
            pos.x + (sz.x - tbSize.x) / 2.f - tbPos.x,
            pos.y + (sz.y - tbSize.y) / 2.f - tbPos.y
        });

        window.draw(t);
    };



    drawBtn(gConsole.btnEnter, "Run", true);
    drawBtn(gConsole.btnBack,  "← Back", false);
}


int main(){
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{1100u, 680u}), "Warehouse Dashboard (SFML)");
    window.setFramerateLimit(60);

    sf::Font font;
    if(!font.openFromFile("arial.ttf")){ cerr<<"Khong mo duoc font\n"; return 1; }

    // Nạp dữ liệu
    auto warehouses = loadWarehouses("warehouses.csv");
    auto receiptsIn = loadReceipts("receipts_in.csv");
    auto receiptsOut = loadReceipts("receipts_out.csv");
    auto inventory = loadReceipts("inventory.csv");
    auto accounts  = loadAccounts("accounts.csv");
    auto history   = loadHistory("history.csv");
    auto whMeta    = loadWhMeta("warehouse_meta.csv"); // id,status,createdAt,createdBy

    // Đếm in/out theo kho
    map<string,int> inCount, outCount;
    for(const auto& r: receiptsIn)  inCount[r.whId]++;
    for(const auto& r: receiptsOut) outCount[r.whId]++;

    // --------- Màn hình chọn quyền ---------
    bool roleChosen=false, isAdmin=false;
    sf::RectangleShape btnAdmin(sf::Vector2f(200.f,60.f));
    btnAdmin.setFillColor(sf::Color(100,180,100));
    btnAdmin.setPosition(sf::Vector2f(300.f,300.f));
    sf::Text txtAdmin(font, "Admin", 20); txtAdmin.setPosition(sf::Vector2f(360.f,315.f));

    sf::RectangleShape btnStaff(sf::Vector2f(200.f,60.f));
    btnStaff.setFillColor(sf::Color(100,100,180));
    btnStaff.setPosition(sf::Vector2f(600.f,300.f));
    sf::Text txtStaff(font, "Staff", 20); txtStaff.setPosition(sf::Vector2f(660.f,315.f));

    while(window.isOpen() && !roleChosen){
        while(auto e = window.pollEvent()){
            if(e->is<sf::Event::Closed>()) window.close();
            if(const auto* mb = e->getIf<sf::Event::MouseButtonPressed>()){
                sf::Vector2f mp(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));

                    // ==== [ADD] Ưu tiên xử lý màn Chi tiết kho (Back / Popup Xóa) ====
                    if (whView==WhView::Detail) {
                        // click trong màn chi tiết
                        sf::Vector2f mp2((float)mb->position.x,(float)mb->position.y);

                        // Back
                        if (gHitBack.contains(mp2)) {
                            showDelConfirm = false;
                            whView = WhView::List;
                            continue;
                        }

                        ;
                    
                    // ==== [ADD] Nếu đang ở trang Detail: chỉ xử lý nút Back ====
                    if (whView == WhView::Detail) {
                        if (gHitBack.contains(mp2)) { whView = WhView::List; }
                        // Bỏ qua các xử lý còn lại khi đang ở Detail
                    } else
                    // ==== [ADD] Đang ở List: click 1 hàng để mở Detail ====
                    {
                        for (auto& t : whRowHitsFull) {
                            const auto& bounds = get<0>(t);
                            if (bounds.contains(mp2)) {
                                selWhId      = get<1>(t);
                                selWhName    = get<2>(t);
                                selWhManager = get<3>(t);
                                whView = WhView::Detail;
                                break;
                            }
                        }
                    }

                    if(hitReload.contains(mp2)){ whLoading=true; whError=false; whLoadClock.restart(); }
                    if(hitFilter.contains(mp2)){
                        whFilter = (whFilter==WhFilter::All)?WhFilter::Active:
                                   (whFilter==WhFilter::Active)?WhFilter::Suspended:WhFilter::All;
                        currPage=1;
                    }
                    if(hitAdd.contains(mp2)){ /* TODO: modal thêm kho */ }
                    if(hitHeaderName.contains(mp2))   toggleSort(WhSort::Name);
                    if(hitHeaderIn.contains(mp2))     toggleSort(WhSort::InCount);
                    if(hitHeaderOut.contains(mp2))    toggleSort(WhSort::OutCount);
                    if(hitHeaderStatus.contains(mp2)) toggleSort(WhSort::Status);
                    if(hitHeaderDate.contains(mp2))   toggleSort(WhSort::CreatedAt);

                    if(hitPgPrev.contains(mp2) && currPage>1) currPage--;
                    if(hitPgNext.contains(mp2)) currPage++; // sẽ clamp ở render
                    if(hitPgSize10.contains(mp2)){ pageSize=10; currPage=1; }
                    if(hitPgSize25.contains(mp2)){ pageSize=25; currPage=1; }
                    if(hitPgSize50.contains(mp2)){ pageSize=50; currPage=1; }

                    (void)hitReload;(void)hitFilter;(void)hitAdd;
                    (void)hitHeaderName;(void)hitHeaderIn;(void)hitHeaderOut;(void)hitHeaderStatus;(void)hitHeaderDate;
                    (void)hitPgPrev;(void)hitPgNext;(void)hitPgSize10;(void)hitPgSize25;(void)hitPgSize50;
                }
            }
            if(const auto* te = e->getIf<sf::Event::TextEntered>()){
                if(typing){
                    if(te->unicode==8 && !searchText.empty()) searchText.pop_back();
                    else if(te->unicode==13) typing=false;
                    else if(te->unicode>=32 && te->unicode<127)
                        searchText.push_back(static_cast<char>(te->unicode));
                }
            }
            if (const auto* mw = e->getIf<sf::Event::MouseWheelScrolled>()){
                // scroll cho các bảng list (giữ code cũ)
                scrollOffset += mw->delta * -30.f;
                if (scrollOffset < 0.f) scrollOffset = 0.f;

                // scroll cho trang chi tiết kho (Admin)
                if (current==Tab::Kho && isAdmin && whView==WhView::Detail){
                    detailScroll += mw->delta * -40.f;   // lăn xuống -> tăng scroll
                    if (detailScroll < 0.f) detailScroll = 0.f;
                    if (detailScroll > detailMaxScroll) detailScroll = detailMaxScroll;
                }
        }

        // bubble animation
        bubbleX += (targetX - bubbleX) * 0.2f;
        bubble.setPosition(sf::Vector2f(bubbleX, navBar.getPosition().y - 1.f));

        window.clear(sf::Color(245,230,140));

        // ==== View khác Dashboard ====
        if(gView == View::Profile){
            drawProfile(window, font, gProfiles, gPermissions, warehouses, loginUser, isAdmin,
                        navBar, bubble, navCount, labels);
            window.display();
            continue;
        }
        if(gView == View::Permission && isAdmin){
            drawPermission(window, font, accounts, warehouses, gPermissions, gPermUI);
            window.display();
            continue;
        }
        if(gView == View::Console){
            drawConsole(window, font, receiptsIn, receiptsOut, inCount, outCount);
            window.display();
            continue;
        }


        // search
        window.draw(searchBox);
        if(searchText.empty()) window.draw(searchHint);
        else { searchTextDraw.setString(searchText); window.draw(searchTextDraw); }

        // build headers/rows
        vector<string> headers;
        vector<vector<string>> rows;

        if(current==Tab::Kho){
            if(isAdmin){
                // ==== [ADD] Nếu đang ở màn chi tiết thì vẽ Detail và bỏ qua List ====
                if (whView == WhView::Detail) {
                    drawWarehouseDetail(window, font, selWhId, selWhName, selWhManager,
                                        navBar, bubble, navCount, labels);
                    window.display();
                    continue; // quay lại vòng lặp chính, không render danh sách
                }
                // ===== Title + toolbar =====
                sf::Text titleA(font, "Danh sach kho", 22); titleA.setFillColor(sf::Color::Black);
                titleA.setPosition(sf::Vector2f(30.f, 80.f));
                float right = (float)window.getSize().x - 30.f;

                // Reload
                sf::RectangleShape boxReload(sf::Vector2f(90.f,34.f));
                boxReload.setFillColor(sf::Color(230,240,255));
                boxReload.setOutlineThickness(1.f); boxReload.setOutlineColor(sf::Color(150,170,200));
                boxReload.setPosition(sf::Vector2f(right-90.f-300.f, 76.f));
                sf::Text tReload(font, "Tai lai", 18); tReload.setFillColor(sf::Color(30,80,160));
                tReload.setPosition(sf::Vector2f(boxReload.getPosition().x+16.f, boxReload.getPosition().y+6.f));

                // Filter toggle
                string fLabel = (whFilter==WhFilter::All)?"Tat ca":(whFilter==WhFilter::Active)?"Hoat dong":"Ngung";
                sf::RectangleShape boxFilter(sf::Vector2f(120.f,34.f));
                boxFilter.setFillColor(sf::Color(240,240,240));
                boxFilter.setOutlineThickness(1.f); boxFilter.setOutlineColor(sf::Color(160,160,160));
                boxFilter.setPosition(sf::Vector2f(right-120.f-160.f, 76.f));
                sf::Text tFilter(font, fLabel, 18); tFilter.setFillColor(sf::Color(30,80,160));
                tFilter.setPosition(sf::Vector2f(boxFilter.getPosition().x+12.f, boxFilter.getPosition().y+6.f));

                // + Thêm kho
                sf::RectangleShape boxAdd(sf::Vector2f(140.f,34.f)); boxAdd.setFillColor(sf::Color(60,140,220));
                boxAdd.setPosition(sf::Vector2f(right-140.f, 76.f));
                sf::Text tAdd(font, "+ Them kho", 18); tAdd.setFillColor(sf::Color::White);
                tAdd.setPosition(sf::Vector2f(boxAdd.getPosition().x+18.f, boxAdd.getPosition().y+6.f));

                static sf::FloatRect hitReload, hitFilter, hitAdd;
                hitReload=boxReload.getGlobalBounds(); hitFilter=boxFilter.getGlobalBounds(); hitAdd=boxAdd.getGlobalBounds();

                // ===== Data model (gộp dữ liệu) =====
                struct Row{ string id,name,manager; int inN,outN; string status,createdAt,createdBy; };
                vector<Row> all; all.reserve(warehouses.size());
                for(const auto& w: warehouses){
                    Row r{w.id,w.name,w.manager, inCount[w.id], outCount[w.id], "Hoat dong","",""};
                    auto it=find_if(whMeta.begin(),whMeta.end(),[&](const WhMeta&m){return m.id==w.id;});
                    if(it!=whMeta.end()){
                        r.status = (it->status=="active"||it->status=="hoat_dong")?"Hoat dong":"Ngung";
                        r.createdAt = fmtDate(it->createdAt); r.createdBy = it->createdBy;
                    }
                    all.push_back(r);
                }
                if(whLoading && whLoadClock.getElapsedTime().asMilliseconds()>800) whLoading=false;

                // ===== Filter + search =====
                vector<Row> filtered; filtered.reserve(all.size());
                for(const auto& r: all){
                    if(whFilter==WhFilter::Active && r.status!="Hoat dong") continue;
                    if(whFilter==WhFilter::Suspended && r.status!="Ngung") continue;
                    if(!searchText.empty() && !(icontains(r.name,searchText)||icontains(r.manager,searchText))) continue;
                    filtered.push_back(r);
                }

                // ===== Sort =====
                auto cmp=[&](const Row&a,const Row&b){
                    switch(whSort){
                        case WhSort::Name:      return whAsc ? a.name<b.name : a.name>b.name;
                        case WhSort::InCount:   return whAsc ? a.inN<b.inN : a.inN>b.inN;
                        case WhSort::OutCount:  return whAsc ? a.outN<b.outN : a.outN>b.outN;
                        case WhSort::Status:    return whAsc ? a.status<b.status : a.status>b.status;
                        case WhSort::CreatedAt: return whAsc ? a.createdAt<b.createdAt : a.createdAt>b.createdAt;
                    } return true;
                };
                sort(filtered.begin(), filtered.end(), cmp);

                // ===== Pagination =====
                int total=(int)filtered.size();
                int maxPage=max(1,(total+pageSize-1)/pageSize);
                if(currPage>maxPage) currPage=maxPage;
                int start=(currPage-1)*pageSize, end=min(total,start+pageSize);

                // ===== Headers =====
                bool isMobile = window.getSize().x < 760;
                vector<string> h = isMobile
                    ? vector<string>{"STT","Ten kho","Quan ly","Trang thai"}
                    : vector<string>{"STT","Ten kho","Quan ly","Phieu nhap","Phieu xuat","Trang thai","Ngay tao","Nguoi tao"};
                vector<float> colW = isMobile
                    ? vector<float>{60.f,360.f,250.f,150.f}
                    : vector<float>{60.f,280.f,220.f,140.f,140.f,150.f,160.f,170.f};
                float totalW=0; for(float w: colW) totalW+=w;

                // Draw title + toolbar
                window.draw(titleA);
                window.draw(boxReload); window.draw(tReload);
                window.draw(boxFilter); window.draw(tFilter);
                window.draw(boxAdd);    window.draw(tAdd);

                if(whError){
                    sf::RectangleShape banner(sf::Vector2f((float)window.getSize().x-60.f,36.f));
                    banner.setFillColor(sf::Color(220,80,80)); banner.setPosition(sf::Vector2f(30.f,120.f));
                    sf::Text t(font,"Khong tai duoc danh sach kho. Thu lai.",16);
                    t.setFillColor(sf::Color::White); t.setPosition(sf::Vector2f(42.f,126.f));
                    window.draw(banner); window.draw(t);
                }

                const float topY = whError ? 166.f : 140.f;
                const float rowH = 36.f;

                sf::RectangleShape header(sf::Vector2f(totalW,rowH));
                header.setFillColor(sf::Color(210,210,210));
                header.setPosition(sf::Vector2f(30.f, topY - scrollOffset));
                window.draw(header);

                float x=30.f;
                static sf::FloatRect hitHeaderName, hitHeaderIn, hitHeaderOut, hitHeaderStatus, hitHeaderDate;
                auto drawHeader=[&](const string& text,float w,bool sortable,WhSort key,sf::FloatRect* save){
                    sf::Text t(font,text,16); t.setFillColor(sf::Color::Black);
                    t.setPosition(sf::Vector2f(x+8.f, topY+8.f - scrollOffset));
                    window.draw(t);
                    if(sortable && whSort==key){
                        sf::Text arrow(font, whAsc? "^" : "v", 14);
                        arrow.setFillColor(sf::Color::Black);
                        arrow.setPosition(sf::Vector2f(x+w-14.f, topY+9.f - scrollOffset));
                        window.draw(arrow);
                    }
                    if (sortable && save)
                        *save = sf::FloatRect( sf::Vector2f(x, topY - scrollOffset), sf::Vector2f(w, rowH) );
                    x+=w;
                };
                drawHeader("STT", colW[0], false, WhSort::Name, nullptr);
                drawHeader("Ten kho", colW[1], true, WhSort::Name, &hitHeaderName);
                if(isMobile){
                    drawHeader("Quan ly", colW[2], false, WhSort::Name, nullptr);
                    drawHeader("Trang thai", colW[3], true, WhSort::Status, &hitHeaderStatus);
                }else{
                    drawHeader("Quan ly", colW[2], false, WhSort::Name, nullptr);
                    drawHeader("Phieu nhap", colW[3], true, WhSort::InCount, &hitHeaderIn);
                    drawHeader("Phieu xuat", colW[4], true, WhSort::OutCount, &hitHeaderOut);
                    drawHeader("Trang thai", colW[5], true, WhSort::Status, &hitHeaderStatus);
                    drawHeader("Ngay tao", colW[6], true, WhSort::CreatedAt, &hitHeaderDate);
                    drawHeader("Nguoi tao", colW[7], false, WhSort::Name, nullptr);
                }

                // ===== Rows / skeleton / empty =====
                if(whLoading){
                    for(int i=0;i<8;i++){
                        sf::RectangleShape sk(sf::Vector2f(totalW,rowH));
                        sk.setFillColor(sf::Color(235,235,235));
                        sk.setPosition(sf::Vector2f(30.f, topY+(i+1)*rowH - scrollOffset));
                        window.draw(sk);
                    }
                }else if(total==0){
                    sf::Text empty(font,"Chua co du lieu.",16); empty.setFillColor(sf::Color(90,90,90));
                    empty.setPosition(sf::Vector2f(40.f, topY+rowH+10.f));
                    window.draw(empty);
                }else{
                    sf::Vector2i mpPix = sf::Mouse::getPosition(window);
                    sf::Vector2f mp((float)mpPix.x,(float)mpPix.y);
                    whRowHitsFull.clear();
                    for(int i=start;i<end;i++){
                        int rIdx=i-start; float y=topY+(rIdx+1)*rowH - scrollOffset;

                        sf::RectangleShape row(sf::Vector2f(totalW,rowH));
                        row.setFillColor((i%2==0)?sf::Color(255,255,255):sf::Color(248,248,248));
                        if (sf::FloatRect(sf::Vector2f(30.f, y), sf::Vector2f(totalW, rowH)).contains(mp))
                             row.setFillColor(sf::Color(235,245,255));
                        row.setPosition(sf::Vector2f(30.f,y));
                        window.draw(row);

                        float x2=30.f;
                        // ==== [ADD] Lưu bounds hàng để bắt click mở detail
                        sf::FloatRect rowBounds(sf::Vector2f(30.f, y), sf::Vector2f(totalW, rowH));
                        whRowHitsFull.push_back({rowBounds, filtered[i].id, filtered[i].name, filtered[i].manager});

                        auto drawCell=[&](const string& s,float w){
                            sf::Text t(font,s,15); t.setFillColor(sf::Color::Black);
                            t.setPosition(sf::Vector2f(x2+8.f, y+8.f));
                            window.draw(t); x2+=w;
                        };

                        const auto& r=filtered[i];
                        drawCell(to_string(i+1), colW[0]); // STT
                        drawCell(r.name, colW[1]);         // Tên kho
                        if(isMobile){
                            drawCell(r.manager, colW[2]);
                            // badge
                            sf::RectangleShape badge(sf::Vector2f(90.f,22.f));
                            badge.setPosition(sf::Vector2f(x2+8.f,y+7.f));
                            badge.setFillColor(r.status=="Hoat dong"? sf::Color(120,190,120): sf::Color(200,160,160));
                            window.draw(badge);
                            sf::Text st(font,r.status,14); st.setFillColor(sf::Color::White);
                            st.setPosition(sf::Vector2f(x2+12.f,y+9.f));
                            window.draw(st);
                            x2+=colW[3];
                        }else{
                            drawCell(r.manager, colW[2]);
                            drawCell(to_string(r.inN), colW[3]);
                            drawCell(to_string(r.outN), colW[4]);
                            // badge
                            sf::RectangleShape badge(sf::Vector2f(100.f,22.f));
                            badge.setPosition(sf::Vector2f(x2+8.f,y+7.f));
                            badge.setFillColor(r.status=="Hoat dong"? sf::Color(120,190,120): sf::Color(200,160,160));
                            window.draw(badge);
                            sf::Text st(font,r.status,14); st.setFillColor(sf::Color::White);
                            st.setPosition(sf::Vector2f(x2+12.f,y+9.f));
                            window.draw(st);
                            x2+=colW[5];
                            drawCell(r.createdAt, colW[6]);
                            drawCell(r.createdBy, colW[7]);
                        }
                    }
                }

                // ===== Pagination =====
                float baseY = topY + ((end - start) + 1) * rowH + 10.f;
                sf::RectangleShape prev(sf::Vector2f(60.f, 28.f));
                sf::RectangleShape next(sf::Vector2f(60.f, 28.f));
                prev.setFillColor(sf::Color(235,235,235));
                next.setFillColor(sf::Color(235,235,235));
                prev.setPosition(sf::Vector2f(30.f,  baseY));
                next.setPosition(sf::Vector2f(100.f, baseY));

                sf::Text tp(font, "<", 16); sf::Text tn(font, ">", 16);
                tp.setFillColor(sf::Color::Black); tn.setFillColor(sf::Color::Black);
                tp.setPosition(sf::Vector2f(prev.getPosition().x + 22.f, prev.getPosition().y + 4.f));
                tn.setPosition(sf::Vector2f(next.getPosition().x + 22.f, next.getPosition().y + 4.f));

                sf::Text pg(font, (to_string(currPage) + "/" + to_string(maxPage)), 16);
                pg.setFillColor(sf::Color::Black); pg.setPosition(sf::Vector2f(170.f, baseY + 2.f));

                window.draw(prev); window.draw(next);
                window.draw(tp);   window.draw(tn);   window.draw(pg);

                sf::Text szL(font, "Hien thi:", 16); szL.setFillColor(sf::Color::Black);
                szL.setPosition(sf::Vector2f(260.f, baseY + 2.f)); window.draw(szL);

                static sf::FloatRect hitPgPrev, hitPgNext, hitPgSize10, hitPgSize25, hitPgSize50;
                auto drawSz = [&](int val, float x) -> sf::FloatRect {
                    sf::RectangleShape rct(sf::Vector2f(44.f, 28.f));
                    rct.setPosition(sf::Vector2f(x, baseY));
                    rct.setFillColor(val == pageSize ? sf::Color(60,140,220) : sf::Color(235,235,235));
                    window.draw(rct);
                    sf::Text tx(font, to_string(val), 16);
                    tx.setFillColor(val == pageSize ? sf::Color::White : sf::Color::Black);
                    tx.setPosition(sf::Vector2f(x + 12.f, baseY + 3.f));
                    window.draw(tx);
                    return rct.getGlobalBounds();
                };
                hitPgPrev  = prev.getGlobalBounds();
                hitPgNext  = next.getGlobalBounds();
                hitPgSize10 = drawSz(10, 350.f);
                hitPgSize25 = drawSz(25, 400.f);
                hitPgSize50 = drawSz(50, 450.f);

                if (currPage > maxPage) currPage = maxPage;
            }else{
                // Staff: bảng đơn giản + lọc chuỗi
                headers = {"ID","Ten kho","Quan ly"};
                string q = toLower(searchText);
                for(const auto& w: warehouses){
                    string comb = toLower(w.id + " " + w.name + " " + w.manager);
                    if(q.empty() || comb.find(q)!=string::npos)
                        rows.push_back({w.id,w.name,w.manager});
                }
                drawTable(window, font, sf::Vector2f(30.f,120.f), headers, rows, scrollOffset);
            }
        }else if(current==Tab::Nhap){
            headers = {"ID","Kho","Ngay","So luong"};
            string q = toLower(searchText);
            for(const auto& r: receiptsIn){
                string comb = toLower(r.id + string(" ") + r.whId + " " + r.date + " " + to_string(r.qty));
                if(q.empty() || comb.find(q)!=string::npos)
                    rows.push_back({r.id,r.whId,r.date,to_string(r.qty)});
            }
            drawTable(window, font, sf::Vector2f(30.f,120.f), headers, rows, scrollOffset);
        }else if(current==Tab::Xuat){
            headers = {"ID","Kho","Ngay","So luong"};
            string q = toLower(searchText);
            for(const auto& r: receiptsOut){
                string comb = toLower(r.id + string(" ") + r.whId + " " + r.date + " " + to_string(r.qty));
                if(q.empty() || comb.find(q)!=string::npos)
                    rows.push_back({r.id,r.whId,r.date,to_string(r.qty)});
            }
            drawTable(window, font, sf::Vector2f(30.f,120.f), headers, rows, scrollOffset);
        }else if(current==Tab::Ton){
            headers = {"ID","Kho","Ngay","Ton"};
            string q = toLower(searchText);
            for(const auto& r: inventory){
                string comb = toLower(r.id + string(" ") + r.whId + " " + r.date + " " + to_string(r.qty));
                if(q.empty() || comb.find(q)!=string::npos)
                    rows.push_back({r.id,r.whId,r.date,to_string(r.qty)});
            }
            drawTable(window, font, sf::Vector2f(30.f,120.f), headers, rows, scrollOffset);
        }else if(current==Tab::LichSu){
            headers = {"Thoi gian","User","Hanh dong"};
            string q = toLower(searchText);
            for(const auto& h: history){
                string comb = toLower(h.time + string(" ") + h.user + " " + h.action);
                if(q.empty() || comb.find(q)!=string::npos)
                    rows.push_back({h.time, h.user, h.action});
            }
            drawTable(window, font, sf::Vector2f(30.f,120.f), headers, rows, scrollOffset);
        }

        // Nav bar + icons + labels
        window.draw(navBar);
        window.draw(bubble);
        for(int i=0;i<navCount;i++){
            float cx = navBar.getPosition().x + i*cellW + cellW*0.5f;
            float cy = navBar.getPosition().y + navH*0.55f;
            sf::CircleShape icon(16.f);
            icon.setOrigin(sf::Vector2f(16.f,16.f));
            icon.setFillColor(sf::Color::Transparent);
            icon.setOutlineThickness(2.5f);
            icon.setOutlineColor(sf::Color(220,220,220));
            icon.setPosition(sf::Vector2f(cx,cy));
            window.draw(icon);

            sf::Text t(font, labels[i], 16);
            t.setFillColor(sf::Color(210,210,210));
            auto b = t.getLocalBounds();
            t.setPosition(sf::Vector2f(cx - b.size.x/2.f, cy + 18.f));
            window.draw(t);
        }

        window.display();
    }
    return 0;
}
/*
D:/bin/mingw64_1420/bin/g++ new.cpp -o new.exe -ID:\bin\SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit\SFML-3.0.2\include -LD:\bin\SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit\SFML-3.0.2\lib -lsfml-graphics -lsfml-window -lsfml-system
*/