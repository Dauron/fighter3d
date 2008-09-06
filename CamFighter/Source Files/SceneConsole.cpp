#include "SceneConsole.h"
#include "SceneGame.h"
#include "SceneMenu.h"
#include "SceneTest.h"

#include "../App Framework/Application.h"
#include "../App Framework/Input/InputMgr.h"
#include "../Graphics/OGL/GLAnimSkeletal.h"

#include "../Utils/Debug.h"
#include "../Utils/GraphicsModes.h"

using namespace Scenes;
    
bool SceneConsole :: Create(int left, int top, unsigned int width, unsigned int height, Scene *prevScene)
{
    Scene::Create(left, top, width, height, prevScene);

    InitInputMgr();

    history = '>';
    histLines = 1;
    scroll_v = 0;

    FL_visible = !PrevScene;

    return true;
}

void SceneConsole :: InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(Name);

    im.Key2InputCode_SetIfKeyFree(VK_RETURN, IC_Accept);
    im.Key2InputCode_SetIfKeyFree(VK_ESCAPE, IC_Reject);
    im.Key2InputCode_SetIfKeyFree(VK_F11,    IC_FullScreen);
    im.Key2InputCode_SetIfKeyFree(VK_BACK,   IC_Con_BackSpace);
#ifdef WIN32
    im.Key2InputCode_SetIfKeyFree(VK_OEM_3,  IC_Console);
#else
    im.Key2InputCode_SetIfKeyFree('`',       IC_Console);
#endif
    im.Key2InputCode_SetIfKeyFree(VK_UP,    IC_Con_LineUp);
    im.Key2InputCode_SetIfKeyFree(VK_DOWN,  IC_Con_LineDown);
    im.Key2InputCode_SetIfKeyFree(VK_PRIOR, IC_Con_PageUp);
    im.Key2InputCode_SetIfKeyFree(VK_NEXT,  IC_Con_PageDown);
    im.Key2InputCode_SetIfKeyFree(VK_HOME,  IC_Con_FirstPage);
    im.Key2InputCode_SetIfKeyFree(VK_END,   IC_Con_LastPage);

    im.SetScene(PrevScene->Name);
#ifdef WIN32
    im.Key2InputCode_SetIfKeyFree(VK_OEM_3,  IC_Console);
#else
    im.Key2InputCode_SetIfKeyFree('`',       IC_Console);
#endif
}
    
void SceneConsole :: Destroy()
{
	Scene::Destroy();
    g_FontMgr.Release(font);
    g_FontMgr.Release(font15);
    font = font15 = HFont();
}
   
void SceneConsole :: Enter()
{
    Scene::Enter();

    FL_justOpened = true;
    g_InputMgr.Buffer.clear();
    g_InputMgr.mouseWheel = 0;

    if (FL_visible)
    {
        FL_overlayInput = false;
        FL_overlayClock = false;
    }
    else
    {
        FL_overlayInput = true;
        FL_overlayClock = true;
        PrevScene->Enter();
    }
}

void SceneConsole :: Exit()
{
    if (PrevScene) PrevScene->Exit();
}
    
void SceneConsole :: Resize(int left, int top, unsigned int width, unsigned int height)
{
	if (PrevScene) PrevScene->Resize(left, top, width, height);
	Scene::Resize(left, top, width, height);

	if (!g_FontMgr.IsHandleValid(font))
		font = g_FontMgr.GetFont("Courier New", 12);
    if (!g_FontMgr.IsHandleValid(font15))
		font15 = g_FontMgr.GetFont("Courier New", 15);
	const GLFont* pFont = g_FontMgr.GetFont(font);

	pageSize = (int)(height/2.0f/pFont->LineH()) - 3;
	if (scroll_v > histLines-1-pageSize) scroll_v = histLines-1-pageSize;
}
    
Scene & SceneConsole :: Scene_Set(Scene& scene, bool fl_destroyPrevious)
{
    if (PrevScene && fl_destroyPrevious)
     {
        if (!scene.IsTerminated() || scene.Create(0, 0, Width, Height))
        {
            PrevScene->Exit();
            PrevScene->Destroy();
            delete PrevScene;
            PrevScene = &scene;
            scene.Enter();
        }
     }
     else
     if (!scene.IsTerminated() || scene.Create(0, 0, Width, Height, PrevScene))
     {
         if (PrevScene) PrevScene->Exit();
         PrevScene = &scene;
         scene.Enter();
     }

    if (PrevScene)
    {        
        g_InputMgr.SetScene(PrevScene->Name);
#ifdef WIN32
        g_InputMgr.Key2InputCode_SetIfKeyFree(VK_OEM_3, IC_Console);
#else
        g_InputMgr.Key2InputCode_SetIfKeyFree('`',      IC_Console);
#endif
    }

    return *this;
}
    
    
void SceneConsole :: AppendConsole(std::string text)
{
    history += text;

    const char *start = text.c_str();
    const char *end;

    while ((end = strchr(start, '\n')))
    {
        ++histLines;
        start = end+1;
    }
    if (scroll_v < histLines -1-pageSize) scroll_v = histLines-1-pageSize;
}

bool SceneConsole :: Update(float T_delta)
{
    float T_tick = GetTick();
    if (T_tick - T_carretTick > 500.f)
    {
        T_carretTick = T_tick;
        FL_carretVisible = !FL_carretVisible;
    }

    InputMgr &im = g_InputMgr;

    if (im.GetInputStateAndClear(IC_Console))
    {
        if (FL_visible && !FL_overlayInput && PrevScene)
        {
            FL_overlayInput = true;
            FL_overlayClock = true;
            PrevScene->Enter();
        }
        else
        {
            FL_visible = true;
            Enter();
        }
        return true;
    }
    else
    if (im.GetInputStateAndClear(IC_Reject))
    {
        if (PrevScene)
        {
            if (FL_visible)
            {
                FL_visible = false;
                PrevScene->Enter();
            }
            else
            {
                im.SetInputState(IC_Reject, true);
                return PrevScene->Update(0.f);
            }
        }
        else
            g_Application.Destroy();
        return true;
    }

    if (!FL_visible)
    {
        im.SetScene(PrevScene->Name);
        PrevScene->Update(T_delta);
        return true;
    }

    if (FL_overlayClock)
    {
        im.SetScene(PrevScene->Name, !FL_overlayInput);
        bool res = PrevScene->Update(T_delta);
        if (FL_overlayInput)
            return res;
        im.SetScene(Name, false);
    }

    im.SetScene(Name);
    if (im.GetInputStateAndClear(IC_FullScreen))
    {
        if (g_Application.MainWindow_Get().IsFullScreen())
            g_Application.MainWindow_Get().FullScreen_Set(Config::WindowX, Config::WindowY, false);
        else
            g_Application.MainWindow_Get().FullScreen_Set(Config::FullScreenX, Config::FullScreenY, true);
        return true;
    }
    else
    if (im.GetInputStateAndClear(IC_Accept))
    {
        AppendConsole(currCmd);
        std::string output;
        bool result = ShellCommand(currCmd, output);
        if (PrevScene) result |= PrevScene->ShellCommand(currCmd, output);
        if (!result)
            AppendConsole("\nUnknown command: " + currCmd);
        else
            AppendConsole(output);

        if (history.size())
            AppendConsole("\n>");
        else
            AppendConsole(">");

        currCmd.clear();
    }
    else
    if (im.GetInputState(IC_Con_LineUp) || im.mouseWheel > 0)
    {
        if (im.mouseWheel > 100) im.mouseWheel -= 100;
        else                     im.mouseWheel = 0;
        if (scroll_v) --scroll_v;
        else          im.mouseWheel = 0;
    }
    else
    if (im.GetInputState(IC_Con_LineDown) || im.mouseWheel < 0)
    {
        if (im.mouseWheel < -100) im.mouseWheel += 100;
        else                      im.mouseWheel = 0;
        if (scroll_v < histLines -1 -pageSize) ++scroll_v;
        else                                   im.mouseWheel = 0;
    }
    else
    if (im.GetInputStateAndClear(IC_Con_PageUp))
    {
        if (scroll_v > pageSize) scroll_v -= pageSize;
        else                     scroll_v = 0;
    }
    else
    if (im.GetInputStateAndClear(IC_Con_PageDown))
    {
        if (scroll_v < histLines -1 -2*pageSize) scroll_v += pageSize;
        else                                     scroll_v = histLines-1-pageSize;
    }
    else
    if (im.GetInputStateAndClear(IC_Con_FirstPage))
        scroll_v = 0;
    else
    if (im.GetInputStateAndClear(IC_Con_LastPage))
        scroll_v = histLines-1-pageSize;
    else if (im.GetInputStateAndClear(IC_Con_BackSpace))
    {
        if (currCmd.length())
            currCmd.erase(currCmd.end()-1);
    }
    else if (g_InputMgr.Buffer.length())
    {
        if (FL_justOpened) // skip the key that has opened the console
            FL_justOpened = false;
        else
            currCmd += g_InputMgr.Buffer;
        g_InputMgr.Buffer.clear();
        if (scroll_v < histLines -1-pageSize) scroll_v = histLines-1-pageSize;
    }

    return true;
}

bool SceneConsole :: ShellCommand(std::string &cmd, std::string &output)
{
    if (cmd == "?" || cmd == "help")
    {
        output.append("\n\
  Available shell comands for [console]:\n\
    Full command        | Short command | Description\n\
    ------------------------------------------------------------------------\n\
    help                | ?             | print this help screen\n\
    clear_console       | clr           | clear console history\n\
    zero_fps_counters   | clrfps        | clears min & max fps counters\n\
    terminate           | qqq           | terminate application\n\
    `                   | `             | enable full overlay mode\n\
    toggle_clock        | tcl           | toggles the clock overlay mode\n\
    ------------------------------------------------------------------------\n\
    graphical_mode_list | gml           | list possible pixel formats\n\
    opengl_extensions   | ext           | list available OpenGL extensions\n\
    status              | status        | show execution status\n\
    ------------------------------------------------------------------------\n\
    log message         | log message   | adds given message to the log file\n\
    log_tail            | tail          | displays tail of the log file\n\
    log_read            | read          | displays the log file\n\
    log_clear           | clrlog        | remove the log file\n");
        return true;
    }
    if (cmd == "clr" || cmd == "clear_console")
    {
        history.clear();
        histLines = 1;
        scroll_v = 0;
        return true;
    }
    if (cmd == "qqq" || cmd == "terminate")
    {
        g_Application.Destroy();
        return true;
    }
    if (cmd == "tcl" || cmd == "toggle_clock")
    {
        FL_overlayClock = !FL_overlayClock;
        if (FL_overlayClock)
            output.append("\nThe clock is ON.\n");
        else
            output.append("\nThe clock is OFF.\n");
        return true;
    }
    if (cmd == "gml" || cmd == "graphical_mode_list")
    {
#ifdef WIN32
        GraphicsModes gm(g_Application.MainWindow_Get().HDC());

        int gm_count = gm.CountModes();
        PIXELFORMATDESCRIPTOR* pfds = gm.ListModes();

        for (int i=0; i<gm_count; ++i)
        {
            output.append("\n  Pixel Format " + itos(i+1) + '\n');
            output.append(gm.ModeToString(pfds[i]));
        }

        delete[] pfds;
#else
        output.append("\nImplemented only for Windows\n");
#endif
        return true;
    }
    if (cmd == "ext" || cmd == "opengl_extensions")
    {
        char *extensions = strdup((char *) glGetString(GL_EXTENSIONS)); // Fetch Extension String
        int len=strlen(extensions);
        for (int i=0; i<len; i++)              // Separate It By Newline Instead Of Blank
            if (extensions[i]==' ') extensions[i]='\n';
        output.append("\nAvailable OpenGL extensions:\n");
        output.append(extensions);
        delete[] extensions;

        return true;
    }
    if (cmd == "status")
    {
    std::stringstream ss;
    std::string txt = "\n\
speed = *", txt2;
    ss << Config::Speed; ss >> txt2; txt += txt2 + "\n\
test  = ";
    ss.clear();
    ss << Config::TestCase;  ss >> txt2; txt += txt2 + '\n';
    AppendConsole(txt);
    if (g_AnimSkeletal.HardwareEnabled())
        output.append("hardware skeletal animation ENABLED\n");
    else
        output.append("hardware skeletal animation DISABLED\n");
    return true;
    }
    if (cmd == "clrfps" || cmd == "zero_fps_counters")
    {
        Performance.FPSmin = 1000.f;
        Performance.FPSmax = 0;
        return true;
    }
    if (cmd.substr(0, 4) == "log ")
    {
        logEx(0, true, cmd.substr(4).c_str());
        return true;
    }
    if (cmd == "tail" || cmd == "log_tail")
    {
        char *res = log_tail();
        output.append("\nLog tail:\n");
        output.append(res);
        return true;
    }
    if (cmd == "read" || cmd == "log_read")
    {
        char *res = log_read();
        output.append("\nLog file:\n");
        output.append(res);
        return true;
    }
    if (cmd == "clrlog" || cmd == "log_clear")
    {
        log_clear();
        return true;
    }
    if (cmd.substr(0, 6) == "scene ")
    {
        if (PrevScene)
        {
            PrevScene->Destroy();
            delete PrevScene;
            PrevScene = NULL;
        }
        if (cmd == "scene test") PrevScene = new SceneTest();
        else
        if (cmd == "scene game") PrevScene = new SceneGame();
        else
        if (cmd == "scene menu") PrevScene = new SceneMenu();
        PrevScene->Create(Left, Top, Width, Height);
        if (FL_visible)
        {
            FL_overlayClock = false;
            FL_overlayInput = false;
            g_InputMgr.SetScene(Name);
        }
        return true;
    }
    return false;
}
    
bool SceneConsole::Render()
{
    if (PrevScene) PrevScene->Render();
    if (!FL_visible) return true;

    GLint cHeight = Height/2;

    glViewport(Left, Top+cHeight, Width, cHeight); // Set viewport
    glDisable(GL_DEPTH_TEST);                      // Disable depth testing
    GLShader::SetLightType(xLight_NONE);
    GLShader::EnableTexturing(xState_Off);
    glDisable (GL_POLYGON_SMOOTH);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_BLEND);                    // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width, 0, cHeight, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const GLFont* pFont = g_FontMgr.GetFont(font);

    float lineHeight = pFont->LineH();

    // Draw backgroud
    glColor4f( 0.0f, 0.0f, 0.0f, 0.5f );
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f((GLfloat)Width, 0.0f);
        glVertex2f((GLfloat)Width, (GLfloat)cHeight);
        glVertex2f(0.0f, (GLfloat)cHeight);
    glEnd();

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    pFont->PrintF(0.0f, (float)cHeight-lineHeight, 0.0f,
        "Console    MinFPS: %u MeanFPS: %2u MaxFPS: %u T_world: %4.3f, T1: %2.2f, T2: %2.2f",
        (int)Performance.FPSmin, (int)Performance.FPSsnap, (int)Performance.FPSmax,
        Performance.T_world,
        Performance.snapCollisionDataFillMS, Performance.snapCollisionDeterminationMS);
    pFont->PrintF(0.0f, (float)cHeight-2*lineHeight, 0.0f,
        "   Num culled elements: %3u diffuse: %u shadows: %u culled: %u zP: %u zF: %u zFs: %u zFf: %u zFb: %u",
        (int)Performance.CulledElements, Performance.CulledDiffuseElements,
        Performance.Shadows.shadows, Performance.Shadows.culled, Performance.Shadows.zPass, Performance.Shadows.zFail,
        Performance.Shadows.zFailS, Performance.Shadows.zFailF, Performance.Shadows.zFailB);

	glScissor(0, cHeight, Width, cHeight);                 // Define Scissor Region
    glEnable(GL_SCISSOR_TEST);                             // Enable Scissor Testing

    pFont->Print(0.0f, cHeight-3*lineHeight, 0.0f, cHeight-3*lineHeight, scroll_v, history.c_str());
    pFont->Print(currCmd.c_str());
    if (FL_carretVisible && scroll_v >= histLines -1-pageSize)
        pFont->Print("_");

    glDisable(GL_SCISSOR_TEST);                            // Disable Scissor Testing

    // Draw scrollbar
    float size = (float)(pageSize+1) / histLines;
    if (size < 1)
    {
        size *= cHeight-2*lineHeight;

        float position = (float)(scroll_v) / histLines;
        position = (cHeight-2*lineHeight)*(1-position);

        glColor4f( 1.0f, 1.0f, 1.0f, 0.5f );
        glBegin(GL_QUADS);
            glVertex2f((GLfloat)Width-10.0f, 0.0f);
            glVertex2f((GLfloat)Width, 0.0f);
            glVertex2f((GLfloat)Width, (GLfloat)cHeight-2*lineHeight);
            glVertex2f(Width-10.0f, (GLfloat)cHeight-2*lineHeight);

            glVertex2f((GLfloat)Width-10.0f, position-size);
            glVertex2f((GLfloat)Width, position-size);
            glVertex2f((GLfloat)Width, position);
            glVertex2f(Width-10.0f, position);
        glEnd();
    }

    glDisable(GL_BLEND);

    // Flush the buffer to force drawing of all objects thus far
    glFlush();

    return true;
}
