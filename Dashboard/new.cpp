#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <ctime>
using namespace std;

// ====== Data models ======
struct Warehouse { string id, name, manager; };   g++ -g new.cpp -o main.exe -I"[SFML_INCLUDE_PATH]" -L"[SFML_LIB_PATH]" -lsfml-graphics -lsfml-window -lsfml-system// Kho
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

    // Title
    sf::Text title(font, ("Kho • " + whName), 26);
    title.setFillColor(sf::Color(25,42,60));
    title.setPosition({30.f, 20.f});
    window.draw(title);

    // Back button
    sf::RectangleShape backBtn({120.f,34.f});
    backBtn.setFillColor(sf::Color(230,240,255));
    backBtn.setOutlineThickness(1.f);
    backBtn.setOutlineColor(sf::Color(160,170,200));
    backBtn.setPosition({30.f, 64.f});
    sf::Text backTxt(font, "← Quay lai", 18);
    backTxt.setFillColor(sf::Color(30,80,160));
    backTxt.setPosition(backBtn.getPosition() + sf::Vector2f{12.f,6.f});
    window.draw(backBtn); window.draw(backTxt);
    gHitBack = backBtn.getGlobalBounds(); // expose cho event loop

    // Thẻ thông tin (trái)
    sf::RectangleShape meta({700.f, 110.f});
    meta.setFillColor(sf::Color(250,250,250));
    meta.setOutlineThickness(1.f); meta.setOutlineColor(sf::Color(200,200,200));
    meta.setPosition({30.f, 112.f});
    window.draw(meta);

    sf::Text tInfo(font,
        "Nguoi tao: FW.SP.MN\nNgay tao: --/--/----\nQuan ly: " + whManager,
        16);
    tInfo.setFillColor(sf::Color(60,60,60));
    tInfo.setPosition(meta.getPosition() + sf::Vector2f{16.f,14.f});
    window.draw(tInfo);

    // KPI (phải)
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
    kpiCard({760.f,112.f}, "So phieu nhap", "0");
    kpiCard({760.f,170.f}, "So phieu xuat", "0");
    kpiCard({760.f,228.f}, "Hang hoa ton kho", "0");

    // Chart
    sf::Text chartTitle(font, "Tinh hinh nhap, xuat kho 12 thang gan day", 18);
    chartTitle.setFillColor(sf::Color(25,42,60));
    chartTitle.setPosition({30.f, 240.f});
    window.draw(chartTitle);

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
                if(btnAdmin.getGlobalBounds().contains(mp)){ isAdmin=true;  roleChosen=true; }
                if(btnStaff.getGlobalBounds().contains(mp)){ isAdmin=false; roleChosen=true; }
            }
        }
        window.clear(sf::Color(245,230,140));
        window.draw(btnAdmin); window.draw(txtAdmin);
        window.draw(btnStaff); window.draw(txtStaff);
        window.display();
    }

    // Toast ngắn sau chọn role
    sf::Clock uiClock;
    const float toastDuration = 2.2f;
    bool showToast = true;
    sf::RectangleShape toastBg({520.f, 80.f});
    toastBg.setFillColor(sf::Color(25, 42, 60, 230));
    toastBg.setPosition({
        window.getSize().x * 0.5f - toastBg.getSize().x * 0.5f,
        window.getSize().y * 0.25f
    });
    sf::Text toastTxt(font, string("Ban da chon: ") + (isAdmin ? "Admin" : "Staff"), 20);
    toastTxt.setFillColor(sf::Color::White);
    toastTxt.setPosition(sf::Vector2f(toastBg.getPosition().x + 20.f, toastBg.getPosition().y + 26.f));

    while(window.isOpen() && showToast){
        while(auto e = window.pollEvent()){
            if(e->is<sf::Event::Closed>()) { window.close(); break; }
        }
        if(!window.isOpen()) return 0;
        if(uiClock.getElapsedTime().asSeconds() >= toastDuration) showToast = false;
        window.clear(sf::Color(245,230,140));
        window.draw(toastBg); window.draw(toastTxt);
        window.display();
    }
    if(!window.isOpen()) return 0;

    // === MÀN HÌNH ĐĂNG NHẬP ===
    bool loggedIn = false;
    string loginUser, loginPass;
    bool focusUser = true, focusPass = false;
    bool showPass = false, capsOn = false;
    string inlineError;
    bool authPending = false; sf::Clock authClock;

    sf::Text title(font, "Dang nhap he thong", 26); title.setFillColor(sf::Color::Black);

    sf::RectangleShape form({520.f, 360.f});
    form.setFillColor(sf::Color(250,250,250));
    form.setOutlineThickness(2.f); form.setOutlineColor(sf::Color(60,60,60));
    form.setPosition({
        window.getSize().x * 0.5f - form.getSize().x * 0.5f,
        window.getSize().y * 0.5f - form.getSize().y * 0.5f
    });
    title.setPosition(sf::Vector2f(form.getPosition().x + 24.f, form.getPosition().y + 18.f));

    sf::Text lblUser(font, "Username", 16); lblUser.setFillColor(sf::Color(70,70,70));
    sf::RectangleShape boxUser({form.getSize().x - 2*30.f, 42.f});
    boxUser.setPosition(sf::Vector2f(form.getPosition().x + 30.f, form.getPosition().y + 78.f));
    boxUser.setFillColor(sf::Color(240,240,240)); boxUser.setOutlineThickness(1.f);
    boxUser.setOutlineColor(sf::Color(150,150,150));
    lblUser.setPosition(sf::Vector2f(boxUser.getPosition().x, boxUser.getPosition().y - 22.f));
    sf::Text txtUser(font, "", 18); txtUser.setFillColor(sf::Color::Black);

    sf::Text lblPass(font, "Password", 16); lblPass.setFillColor(sf::Color(70,70,70));
    sf::RectangleShape boxPass({form.getSize().x - 2*30.f, 42.f});
    boxPass.setPosition(sf::Vector2f(form.getPosition().x + 30.f, form.getPosition().y + 156.f));
    boxPass.setFillColor(sf::Color(240,240,240)); boxPass.setOutlineThickness(1.f);
    boxPass.setOutlineColor(sf::Color(150,150,150));
    lblPass.setPosition(sf::Vector2f(boxPass.getPosition().x, boxPass.getPosition().y - 22.f));
    sf::Text txtPass(font, "", 18); txtPass.setFillColor(sf::Color::Black);

    // nút 👁 show/hide
    sf::RectangleShape btnEye({40.f, 42.f});
    btnEye.setPosition(sf::Vector2f(boxPass.getPosition().x + boxPass.getSize().x - 40.f, boxPass.getPosition().y));
    btnEye.setFillColor(sf::Color(230,230,230)); btnEye.setOutlineThickness(1.f);
    btnEye.setOutlineColor(sf::Color(150,150,150));
    sf::Text eyeTxt(font, "👁", 18); eyeTxt.setFillColor(sf::Color::Black);
    eyeTxt.setPosition(sf::Vector2f(btnEye.getPosition().x + 10.f, btnEye.getPosition().y + 8.f));

    // link + cảnh báo
    sf::Text forgot(font, "Quen mat khau?", 16); forgot.setFillColor(sf::Color(60,120,200));
    forgot.setPosition(sf::Vector2f(boxPass.getPosition().x, boxPass.getPosition().y + 50.f));
    sf::Text capsWarn(font, "Ban dang bat Caps Lock.", 16); capsWarn.setFillColor(sf::Color(180,40,40));
    capsWarn.setPosition(sf::Vector2f(boxPass.getPosition().x, forgot.getPosition().y + 24.f));

    // nút Đăng nhập
    sf::RectangleShape btnLogin({180.f, 46.f});
    btnLogin.setPosition(sf::Vector2f(
        form.getPosition().x + form.getSize().x - 30.f - btnLogin.getSize().x,
        form.getPosition().y + form.getSize().y - 30.f - btnLogin.getSize().y));
    btnLogin.setFillColor(sf::Color(60,140,220));
    sf::Text btnLoginTxt(font, "Dang nhap", 18); btnLoginTxt.setFillColor(sf::Color::White);
    btnLoginTxt.setPosition(sf::Vector2f(btnLogin.getPosition().x + 36.f, btnLogin.getPosition().y + 11.f));

    // lỗi inline
    sf::Text errTxt(font, "", 16); errTxt.setFillColor(sf::Color(180,40,40));
    sf::Text warnIcon(font, "⚠", 16); warnIcon.setFillColor(sf::Color(180,40,40));

    auto applyFocusOutline = [&]() { // viền xanh khi focus
        boxUser.setOutlineColor(focusUser ? sf::Color(60,120,200) : sf::Color(150,150,150));
        boxPass.setOutlineColor(focusPass ? sf::Color(60,120,200) : sf::Color(150,150,150));
    };

    enum class AuthStatusTmp { OK, BAD_CRED, LOCKED, NO_WAREHOUSE, WAREHOUSE_SUSPENDED };
    auto mockLogin = [&](const string& selRole, const string& u, const string& p)->AuthStatusTmp {
        if(u == "locked_user") return AuthStatusTmp::LOCKED;
        if(selRole == "staff" && u == "staff_no_wh") return AuthStatusTmp::NO_WAREHOUSE;
        if(selRole == "staff" && u == "staff_sus")   return AuthStatusTmp::WAREHOUSE_SUSPENDED;

        string neededRole = selRole; transform(neededRole.begin(), neededRole.end(), neededRole.begin(), ::tolower);
        for(const auto& a: accounts){
            if(a.user==u && a.pass==p && a.role==neededRole) return AuthStatusTmp::OK;
        }
        return AuthStatusTmp::BAD_CRED;
    };

    auto submitLogin = [&]() { // Gửi đăng nhập
        inlineError.clear();
        if(loginUser.empty() || loginPass.empty()){ inlineError = "Vui long nhap day du thong tin."; return; }
        if(authPending) return; authPending = true; authClock.restart();
    };

    focusUser = true; focusPass = false; applyFocusOutline();

    while(window.isOpen() && !loggedIn){
        while(auto e = window.pollEvent()){
            if(e->is<sf::Event::Closed>()) { window.close(); break; }
            if(const auto* ke = e->getIf<sf::Event::KeyPressed>()){
                if(ke->scancode == sf::Keyboard::Scancode::CapsLock) capsOn = !capsOn;
                if(ke->scancode == sf::Keyboard::Scancode::Tab){
                    if(focusUser){ focusUser=false; focusPass=true; } else { focusUser=true; focusPass=false; }
                    applyFocusOutline();
                }
            }
            if(const auto* mb = e->getIf<sf::Event::MouseButtonPressed>()){
                sf::Vector2f mp(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
                if(boxUser.getGlobalBounds().contains(mp)){ focusUser=true; focusPass=false; applyFocusOutline(); }
                else if(boxPass.getGlobalBounds().contains(mp)){ focusUser=false; focusPass=true; applyFocusOutline(); }
                if(btnEye.getGlobalBounds().contains(mp)) showPass = !showPass;
                if(btnLogin.getGlobalBounds().contains(mp) && !authPending) submitLogin();
                if(forgot.getGlobalBounds().contains(mp)) inlineError = "Vui long lien he quan tri de dat lai mat khau.";
            }
            if(const auto* te = e->getIf<sf::Event::TextEntered>()){
                if(te->unicode == 8){
                    if(focusUser && !loginUser.empty()) loginUser.pop_back();
                    else if(focusPass && !loginPass.empty()) loginPass.pop_back();
                }else if(te->unicode == 13){
                    if(focusPass) submitLogin();
                    else { focusUser=false; focusPass=true; applyFocusOutline(); }
                }else if(te->unicode >= 32 && te->unicode < 127){
                    char ch = (char)te->unicode;
                    if(focusUser) loginUser.push_back(ch); else if(focusPass) loginPass.push_back(ch);
                    if(!inlineError.empty()) inlineError.clear();
                }
            }
        }
        if(!window.isOpen()) return 0;

        if(authPending && authClock.getElapsedTime().asMilliseconds() >= 600){
            authPending = false;
            string roleStr = isAdmin ? "admin" : "staff";
            auto st = mockLogin(roleStr, loginUser, loginPass);
            switch(st) {
                case AuthStatusTmp::OK: loggedIn = true; break;
                case AuthStatusTmp::BAD_CRED: inlineError="Thong tin dang nhap khong dung. Thu lai."; loginPass.clear(); break;
                case AuthStatusTmp::LOCKED: inlineError="Tai khoan cua ban dang bi khoa. Lien he quan tri."; loginPass.clear(); break;
                case AuthStatusTmp::NO_WAREHOUSE: inlineError="Tai khoan chua duoc gan kho. Lien he quan tri."; loginPass.clear(); break;
                case AuthStatusTmp::WAREHOUSE_SUSPENDED: inlineError="Kho ban phu trach dang ngung hoat dong."; loginPass.clear(); break;
            }
        }

        window.clear(sf::Color(245,230,140));
        window.draw(form); window.draw(title);
        window.draw(boxUser); window.draw(boxPass);
        window.draw(lblUser); window.draw(lblPass);

        sf::Text txtU = txtUser; txtU.setString(loginUser);
        txtU.setPosition(sf::Vector2f(boxUser.getPosition().x + 8.f, boxUser.getPosition().y + 9.f));
        window.draw(txtU);

        sf::Text txtP = txtPass; string passShown = showPass ? loginPass : string(loginPass.size(), '*');
        txtP.setString(passShown);
        txtP.setPosition(sf::Vector2f(boxPass.getPosition().x + 8.f, boxPass.getPosition().y + 9.f));
        window.draw(txtP);

        window.draw(btnEye); window.draw(eyeTxt);
        window.draw(forgot);
        if(focusPass && capsOn) window.draw(capsWarn);

        if(!inlineError.empty()){
            warnIcon.setPosition(sf::Vector2f(boxPass.getPosition().x, btnLogin.getPosition().y - 30.f));
            errTxt.setString(inlineError);
            errTxt.setPosition(sf::Vector2f(warnIcon.getPosition().x + 20.f, warnIcon.getPosition().y - 2.f));
            window.draw(warnIcon); window.draw(errTxt);
        }

        btnLogin.setFillColor(authPending ? sf::Color(150,170,190) : sf::Color(60,140,220));
        window.draw(btnLogin); window.draw(btnLoginTxt);
        window.display();
    }
    if(!window.isOpen()) return 0;
    
    // --------- Dashboard ---------
    Tab current = Tab::Kho;
    string searchText; bool typing=false;
    float scrollOffset = 0.f;

    // Admin/Kho state
    enum class WhSort { Name, InCount, OutCount, Status, CreatedAt };
    WhSort whSort = WhSort::Name; bool whAsc = true;
    enum class WhFilter { All, Active, Suspended }; WhFilter whFilter = WhFilter::All;
    int pageSize = 10; const int pageSizeOpts[3]={10,25,50}; (void)pageSizeOpts;
    int currPage = 1;
    bool whLoading=false, whError=false; sf::Clock whLoadClock;

    // Search box
    sf::RectangleShape searchBox(sf::Vector2f(600.f,38.f));
    searchBox.setFillColor(sf::Color(240,240,240));
    searchBox.setOutlineThickness(1.f); searchBox.setOutlineColor(sf::Color(150,150,150));
    searchBox.setPosition(sf::Vector2f(30.f,20.f));
    sf::Text searchHint(font, "Nhap noi dung can tim...", 16); searchHint.setFillColor(sf::Color(120,120,120));
    searchHint.setPosition(sf::Vector2f(40.f,28.f));
    sf::Text searchTextDraw(font, "", 18); searchTextDraw.setFillColor(sf::Color::Black);
    searchTextDraw.setPosition(sf::Vector2f(40.f,26.f));

    // Nav
    const int navCount = 5;
    const string labels[navCount] = {"Kho hang","Phieu nhap","Phieu xuat","Ton kho","Lich su"};
    const float navH = 90.f;

    sf::RectangleShape navBar(sf::Vector2f(static_cast<float>(window.getSize().x) - 80.f, navH));
    navBar.setFillColor(sf::Color(25,42,60));
    navBar.setPosition(sf::Vector2f(40.f, static_cast<float>(window.getSize().y) - navH - 20.f));
    const float cellW = navBar.getSize().x / navCount;

    sf::CircleShape bubble(32.f); bubble.setFillColor(sf::Color(25,42,60)); bubble.setOrigin(sf::Vector2f(32.f,32.f));
    float bubbleX = navBar.getPosition().x + cellW*0.5f, targetX = bubbleX;

    vector<NavItem> items; items.reserve(navCount);
    for(int i=0;i<navCount;i++){
        items.push_back(NavItem{ labels[i],
            sf::FloatRect( sf::Vector2f(navBar.getPosition().x + i*cellW, navBar.getPosition().y),
                           sf::Vector2f(cellW, navH) )});
    }
    // Hàm chuyển tab
    auto setTab = [&](Tab t){
        current = t; scrollOffset = 0.f;
        int idx = (t==Tab::Kho?0:t==Tab::Nhap?1:t==Tab::Xuat?2:t==Tab::Ton?3:4);
        targetX = navBar.getPosition().x + idx*cellW + cellW*0.5f;
    };

    while(window.isOpen()){
        while(auto e = window.pollEvent()){
            if(e->is<sf::Event::Closed>()) window.close();

            if(const auto* mb = e->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mp(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
                typing=false;
                if(searchBox.getGlobalBounds().contains(mp)) typing=true;
                for(int i=0;i<navCount;i++) if(items[i].hit.contains(mp)) setTab((Tab)i);

                // ==== Nút chuyển view ====
                sf::FloatRect avatarBtn({window.getSize().x - 60.f, 20.f}, {40.f, 40.f});
                if(avatarBtn.contains(mp)){
                    gView = View::Profile;
                }
                sf::FloatRect permBtn  ({window.getSize().x - 180.f, 20.f}, {100.f, 40.f});
                if(permBtn.contains(mp) && isAdmin){
                    gView = View::Permission;
                }
                sf::FloatRect consoleBtn({window.getSize().x - 180.f, 20.f}, {100.f, 40.f});
                if(consoleBtn.contains(mp)){
                    gView = View::Console;
                }


                // Sự kiện riêng cho tab Kho (Admin)
                if(current==Tab::Kho && isAdmin){
                    sf::Vector2f mp2((float)mb->position.x,(float)mb->position.y);
                    static sf::FloatRect hitReload, hitFilter, hitAdd;
                    static sf::FloatRect hitHeaderName, hitHeaderIn, hitHeaderOut, hitHeaderStatus, hitHeaderDate;
                    static sf::FloatRect hitPgPrev, hitPgNext, hitPgSize10, hitPgSize25, hitPgSize50;

                    auto toggleSort=[&](WhSort k){ if(whSort==k) whAsc=!whAsc; else {whSort=k; whAsc=true;} };
                    
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
            if(const auto* mw = e->getIf<sf::Event::MouseWheelScrolled>()){
                scrollOffset += mw->delta * -30.f;
                if(scrollOffset < 0.f) scrollOffset = 0.f;
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