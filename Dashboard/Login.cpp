#include "Login.h"
#include <algorithm>
#include <iostream>

using namespace std;

bool showLoginScreen(sf::RenderWindow& window,
                     sf::Font& font,
                     const vector<Account>& accounts,
                     bool isAdmin)
{
    bool loggedIn = false;
    string loginUser, loginPass;
    bool focusUser = true, focusPass = false;
    bool showPass = false, capsOn = false;
    string inlineError;
    bool authPending = false; sf::Clock authClock;

    // UI elements
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
    boxUser.setPosition({form.getPosition().x + 30.f, form.getPosition().y + 78.f});
    boxUser.setFillColor(sf::Color(240,240,240));
    boxUser.setOutlineThickness(1.f); boxUser.setOutlineColor(sf::Color(150,150,150));
    lblUser.setPosition(boxUser.getPosition().x, boxUser.getPosition().y - 22.f);
    sf::Text txtUser(font, "", 18); txtUser.setFillColor(sf::Color::Black);

    sf::Text lblPass(font, "Password", 16); lblPass.setFillColor(sf::Color(70,70,70));
    sf::RectangleShape boxPass({form.getSize().x - 2*30.f, 42.f});
    boxPass.setPosition({form.getPosition().x + 30.f, form.getPosition().y + 156.f});
    boxPass.setFillColor(sf::Color(240,240,240));
    boxPass.setOutlineThickness(1.f); boxPass.setOutlineColor(sf::Color(150,150,150));
    lblPass.setPosition(boxPass.getPosition().x, boxPass.getPosition().y - 22.f);
    sf::Text txtPass(font, "", 18); txtPass.setFillColor(sf::Color::Black);

    // nút 👁 show/hide
    sf::RectangleShape btnEye({40.f, 42.f});
    btnEye.setPosition({boxPass.getPosition().x + boxPass.getSize().x - 40.f, boxPass.getPosition().y});
    btnEye.setFillColor(sf::Color(230,230,230)); btnEye.setOutlineThickness(1.f);
    btnEye.setOutlineColor(sf::Color(150,150,150));
    sf::Text eyeTxt(font, "👁", 18); eyeTxt.setFillColor(sf::Color::Black);
    eyeTxt.setPosition({btnEye.getPosition().x + 10.f, btnEye.getPosition().y + 8.f});

    // link + cảnh báo
    sf::Text forgot(font, "Quen mat khau?", 16); forgot.setFillColor(sf::Color(60,120,200));
    forgot.setPosition({boxPass.getPosition().x, boxPass.getPosition().y + 50.f});
    sf::Text capsWarn(font, "Ban dang bat Caps Lock.", 16); capsWarn.setFillColor(sf::Color(180,40,40));
    capsWarn.setPosition({boxPass.getPosition().x, forgot.getPosition().y + 24.f});

    // nút Đăng nhập
    sf::RectangleShape btnLogin({180.f, 46.f});
    btnLogin.setPosition({
        form.getPosition().x + form.getSize().x - 30.f - btnLogin.getSize().x,
        form.getPosition().y + form.getSize().y - 30.f - btnLogin.getSize().y
    });
    btnLogin.setFillColor(sf::Color(60,140,220));
    sf::Text btnLoginTxt(font, "Dang nhap", 18); btnLoginTxt.setFillColor(sf::Color::White);
    btnLoginTxt.setPosition({btnLogin.getPosition().x + 36.f, btnLogin.getPosition().y + 11.f});

    // lỗi inline
    sf::Text errTxt(font, "", 16); errTxt.setFillColor(sf::Color(180,40,40));
    sf::Text warnIcon(font, "⚠", 16); warnIcon.setFillColor(sf::Color(180,40,40));

    auto applyFocusOutline = [&](){
        boxUser.setOutlineColor(focusUser ? sf::Color(60,120,200) : sf::Color(150,150,150));
        boxPass.setOutlineColor(focusPass ? sf::Color(60,120,200) : sf::Color(150,150,150));
    };

    enum class AuthStatus { OK, BAD_CRED, LOCKED, NO_WAREHOUSE, WAREHOUSE_SUSPENDED };
    auto mockLogin = [&](const string& selRole, const string& u, const string& p)->AuthStatus {
        if(u == "locked_user") return AuthStatus::LOCKED;
        if(selRole == "staff" && u == "staff_no_wh") return AuthStatus::NO_WAREHOUSE;
        if(selRole == "staff" && u == "staff_sus")   return AuthStatus::WAREHOUSE_SUSPENDED;

        string neededRole = selRole;
        transform(neededRole.begin(), neededRole.end(), neededRole.begin(), ::tolower);
        for(const auto& a: accounts){
            if(a.user==u && a.pass==p && a.role==neededRole) return AuthStatus::OK;
        }
        return AuthStatus::BAD_CRED;
    };

    auto submitLogin = [&](){
        inlineError.clear();
        if(loginUser.empty() || loginPass.empty()){
            inlineError = "Vui long nhap day du thong tin."; return;
        }
        if(authPending) return;
        authPending = true; authClock.restart();
    };

    focusUser = true; focusPass = false; applyFocusOutline();

    // === Vòng lặp chính cho màn hình login ===
    while(window.isOpen() && !loggedIn){
        while(auto e = window.pollEvent()){
            if(e->is<sf::Event::Closed>()){ window.close(); break; }
            if(const auto* ke = e->getIf<sf::Event::KeyPressed>()){
                if(ke->scancode == sf::Keyboard::Scancode::CapsLock) capsOn = !capsOn;
                if(ke->scancode == sf::Keyboard::Scancode::Tab){
                    if(focusUser){ focusUser=false; focusPass=true; }
                    else { focusUser=true; focusPass=false; }
                    applyFocusOutline();
                }
            }
            if(const auto* mb = e->getIf<sf::Event::MouseButtonPressed>()){
                sf::Vector2f mp((float)mb->position.x,(float)mb->position.y);
                if(boxUser.getGlobalBounds().contains(mp)){ focusUser=true; focusPass=false; applyFocusOutline(); }
                else if(boxPass.getGlobalBounds().contains(mp)){ focusUser=false; focusPass=true; applyFocusOutline(); }
                if(btnEye.getGlobalBounds().contains(mp)) showPass = !showPass;
                if(btnLogin.getGlobalBounds().contains(mp) && !authPending) submitLogin();
                if(forgot.getGlobalBounds().contains(mp))
                    inlineError = "Vui long lien he quan tri de dat lai mat khau.";
            }
            if(const auto* te = e->getIf<sf::Event::TextEntered>()){
                if(te->unicode == 8){ // Backspace
                    if(focusUser && !loginUser.empty()) loginUser.pop_back();
                    else if(focusPass && !loginPass.empty()) loginPass.pop_back();
                }else if(te->unicode == 13){ // Enter
                    if(focusPass) submitLogin();
                    else { focusUser=false; focusPass=true; applyFocusOutline(); }
                }else if(te->unicode >= 32 && te->unicode < 127){
                    char ch = (char)te->unicode;
                    if(focusUser) loginUser.push_back(ch);
                    else if(focusPass) loginPass.push_back(ch);
                    if(!inlineError.empty()) inlineError.clear();
                }
            }
        }
        if(!window.isOpen()) return false;

        // xử lý chờ xác thực
        if(authPending && authClock.getElapsedTime().asMilliseconds() >= 600){
            authPending = false;
            string roleStr = isAdmin ? "admin" : "staff";
            auto st = mockLogin(roleStr, loginUser, loginPass);
            switch(st){
                case AuthStatus::OK: loggedIn = true; break;
                case AuthStatus::BAD_CRED: inlineError="Thong tin dang nhap khong dung."; loginPass.clear(); break;
                case AuthStatus::LOCKED: inlineError="Tai khoan bi khoa."; loginPass.clear(); break;
                case AuthStatus::NO_WAREHOUSE: inlineError="Chua duoc gan kho."; loginPass.clear(); break;
                case AuthStatus::WAREHOUSE_SUSPENDED: inlineError="Kho dang ngung hoat dong."; loginPass.clear(); break;
            }
        }

        // vẽ UI
        window.clear(sf::Color(245,230,140));
        window.draw(form); window.draw(title);
        window.draw(boxUser); window.draw(boxPass);
        window.draw(lblUser); window.draw(lblPass);

        sf::Text txtU = txtUser; txtU.setString(loginUser);
        txtU.setPosition(boxUser.getPosition().x + 8.f, boxUser.getPosition().y + 9.f);
        window.draw(txtU);

        sf::Text txtP = txtPass;
        string passShown = showPass ? loginPass : string(loginPass.size(), '*');
        txtP.setString(passShown);
        txtP.setPosition(boxPass.getPosition().x + 8.f, boxPass.getPosition().y + 9.f);
        window.draw(txtP);

        window.draw(btnEye); window.draw(eyeTxt);
        window.draw(forgot);
        if(focusPass && capsOn) window.draw(capsWarn);

        if(!inlineError.empty()){
            warnIcon.setPosition({boxPass.getPosition().x, btnLogin.getPosition().y - 30.f});
            errTxt.setString(inlineError);
            errTxt.setPosition({warnIcon.getPosition().x + 20.f, warnIcon.getPosition().y - 2.f});
            window.draw(warnIcon); window.draw(errTxt);
        }

        btnLogin.setFillColor(authPending ? sf::Color(150,170,190) : sf::Color(60,140,220));
        window.draw(btnLogin); window.draw(btnLoginTxt);

        window.display();
    }

    return loggedIn;
}