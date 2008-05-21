#include "SceneConsole.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../OpenGL/GLAnimSkeletal.h"

#include "../Utils/Debug.h"
#include "../Utils/GraphicsModes.h"
#include "../Models/ModelMgr.h"

bool SceneConsole::Initialize(int left, int top, unsigned int width, unsigned int height)
{
    prevScene->Initialize(left, top, width, height);
    Scene::Initialize(left, top, width, height);

    InitInputMgr();
    g_InputMgr.Buffer.clear();
    justOpened = true;

    history = '>';
    histLines = 1;
    scroll_v = 0;

    return true;
}

void SceneConsole::Resize(int left, int top, unsigned int width, unsigned int height)
{
    prevScene->Resize(left, top, width, height);
    Scene::Resize(left, top, width, height);

    if (!g_FontMgr.IsHandleValid(font))
        font = g_FontMgr.GetFont("Courier New", 12); // it takes about second to create a font
    const GLFont* pFont = g_FontMgr.GetFont(font);

    pageSize = (int)(height/2.0f/pFont->LineH()) - 3;
    if (scroll_v > histLines-1-pageSize) scroll_v = histLines-1-pageSize;
}

void SceneConsole::InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(sceneName);

    im.SetInputCode(VK_RETURN, IC_Accept);
    im.SetInputCode(VK_ESCAPE, IC_Reject);
    im.SetInputCode(VK_F11,    IC_FullScreen);
    im.SetInputCode(VK_BACK,   IC_Con_BackSpace);
#ifdef WIN32
    im.SetInputCode(VK_OEM_3,  IC_Console);
#else
    im.SetInputCode('`',       IC_Console);
#endif
    im.SetInputCode(VK_UP,    IC_Con_LineUp);
    im.SetInputCode(VK_DOWN,  IC_Con_LineDown);
    im.SetInputCode(VK_PRIOR, IC_Con_PageUp);
    im.SetInputCode(VK_NEXT,  IC_Con_PageDown);
    im.SetInputCode(VK_HOME,  IC_Con_FirstPage);
    im.SetInputCode(VK_END,   IC_Con_LastPage);
}

void SceneConsole::Terminate()
{
    g_FontMgr.DeleteFont(font);
    font = HFont();
    if (prevScene)
    {
        prevScene->Terminate();
        delete prevScene;
        prevScene = NULL;
    }
}

void SceneConsole::AppendConsole(std::string text)
{
    history += text;

    const char *start = text.data();
    const char *end;

    while ((end = strchr(start, '\n')))
    {
        ++histLines;
        start = end+1;
    }
    if (scroll_v < histLines -1-pageSize) scroll_v = histLines-1-pageSize;
}

bool SceneConsole::Update(float deltaTime)
{
    if (deltaTime != 0.0f) {
        curFPS = (unsigned int)(1/deltaTime);
        if (curFPS > maxFPS) maxFPS = curFPS;
        if (curFPS < minFPS && curFPS > 0) minFPS = curFPS;
        if (fabs(meanFPS - (float)curFPS) > 5.0f)
            meanFPS = ((meanFPS*19.0f + curFPS)/20.0f);
        else
            meanFPS = ((meanFPS*49.0f + curFPS)/50.0f);
        if (meanFPS > maxFPS) meanFPS = (float)maxFPS;
        if (meanFPS < minFPS) meanFPS = (float)minFPS;
    }
    float curTick = GetTick();
    if (curTick - carretTick > 500)
    {
        carretTick = curTick;
        carretVisible = !carretVisible;
    }

    InputMgr &im = g_InputMgr;

    if (im.GetInputStateAndClear(IC_Console))
    {
        overlayInput = !overlayInput;
        overlayClock = overlayInput;
        if (overlayInput)
            im.SetScene(prevScene->sceneName);
        else
        {
            im.SetScene(sceneName);
            justOpened = true;
        }
        return true;
    }

    if (overlayClock)
    {
        im.enable = overlayInput;
        bool res = prevScene->Update(deltaTime);
        if (overlayInput)
            return res;
        else
            im.enable = true;
    }

    if (im.GetInputStateAndClear(IC_Reject))
    {
        Scene *tmp = prevScene;
        prevScene = NULL;
        g_Application.SetCurrentScene(tmp);
    }
    else
    if (im.GetInputStateAndClear(IC_FullScreen))
        g_Application.MainWindow().SetFullScreen(!g_Application.MainWindow().FullScreen());
    else
    if (im.GetInputStateAndClear(IC_Accept))
    {
        AppendConsole(currCmd);
        if (!ProcessCmd(currCmd))
            AppendConsole("\nUnknown command: " + currCmd);

        if (history.size())
            AppendConsole("\n>");
        else
            AppendConsole(">");

        currCmd.clear();
    }
    else
    if (im.GetInputState(IC_Con_LineUp))
    {
        if (scroll_v) --scroll_v;
    }
    else
    if (im.GetInputState(IC_Con_LineDown))
    {
        if (scroll_v < histLines -1 -pageSize) ++scroll_v;
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
        if (justOpened) // skip the key that has opened the console
        {
            g_InputMgr.Buffer.clear();
            justOpened = false;
        }
        else
        {
            currCmd += g_InputMgr.Buffer;
            g_InputMgr.Buffer.clear();
        }
        if (scroll_v < histLines -1-pageSize) scroll_v = histLines-1-pageSize;
    }

    return true;
}

bool SceneConsole::ProcessCmd(std::string cmd)
{
    if (cmd == "?" || cmd == "help")
    {
        AppendConsole("\n\
  Available console comands:\n\
    Full command        | Short command | Description\n\
    ------------------------------------------------------------------------\n\
    help                | ?             | print this help screen\n\
    clear_console       | clr           | clear console history\n\
    terminate           | qqq           | terminate application\n\
    `                   | `             | enable full overlay mode\n\
    toggle_clock        | tcl           | toggles the clock overlay mode\n\
    ------------------------------------------------------------------------\n\
    graphical_mode_list | gml           | list possible pixel formats\n\
    opengl_extensions   | ext           | list available OpenGL extensions\n\
    ------------------------------------------------------------------------\n\
    toggle_lights       | tls           | turns the lighting on and off\n\
    reinitialize        | init          | reinitialize objects, etc.\n\
    toggle_shader       | tshd          | toggles custom shader\n\
    toggle_polygon_mode | tpm           | toggle polygon mode\n\
    zero_fps_counters   | clrfps        | clears min & max fps counters\n\
    ------------------------------------------------------------------------\n\
    status              | status        | show execution status\n\
    test 0-1            | test 0-1      | load given test scene\n\
    speed {float}       | speed {float} | enter clock speed multiplier\n\
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
        g_Application.MainWindow().Terminate();
        return true;
    }
    if (cmd == "tcl" || cmd == "toggle_clock")
    {
        overlayClock = !overlayClock;
        if (overlayClock)
            AppendConsole("\nThe clock is ON.\n");
        else
            AppendConsole("\nThe clock is OFF.\n");
        return true;
    }
    if (cmd == "gml" || cmd == "graphical_mode_list")
    {
#ifdef WIN32
        GraphicsModes gm(g_Application.MainWindow().HDC());

        int gm_count = gm.CountModes();
        PIXELFORMATDESCRIPTOR* pfds = gm.ListModes();

        for (int i=0; i<gm_count; ++i)
        {
            AppendConsole("\n  Pixel Format " + itos(i+1) + '\n');
            AppendConsole(gm.ModeToString(pfds[i]));
        }

        delete[] pfds;
#else
        AppendConsole("\nImplemented only for Windows\n");
#endif
        return true;
    }
    if (cmd == "ext" || cmd == "opengl_extensions")
    {
        char *extensions = strdup((char *) glGetString(GL_EXTENSIONS)); // Fetch Extension String
        int len=strlen(extensions);
        for (int i=0; i<len; i++)              // Separate It By Newline Instead Of Blank
            if (extensions[i]==' ') extensions[i]='\n';
        AppendConsole("\nAvailable OpenGL extensions:\n");
        AppendConsole(extensions);
        delete[] extensions;

        return true;
    }
    if (cmd == "tls" || cmd == "toggle_lights")
    {
        if (Config::EnableLighting = !Config::EnableLighting)
            AppendConsole("\nThe lights are ON.\n");
        else
            AppendConsole("\nThe lights are OFF.\n");
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
        AppendConsole("hardware skeletal animation ENABLED\n");
    else
        AppendConsole("hardware skeletal animation DISABLED\n");
    return true;
    }
    if (cmd == "init" || cmd == "reinitialize")
    {
        Config::Initialize = true;
        return true;
    }
    if (cmd.substr(0, 5) == "test ")
    {
        Config::TestCase = atoi(cmd.substr(5).data());
        Config::Initialize = true;
        return true;
    }
    if (cmd.substr(0, 6) == "speed ")
    {
        Config::Speed = atof(cmd.substr(6).data());
        return true;
    }
    if (cmd == "tshd" || cmd == "toggle_shader")
    {
        if (Config::EnableShaders = !Config::EnableShaders)
            AppendConsole("\nThe shaders are ON.\n");
        else
            AppendConsole("\nThe shaders are OFF.\n");
        return true;
    }
    if (cmd == "tpm" || cmd == "toggle_polygon_mode")
    {
        switch (Config::PolygonMode)
        {
            case GL_FILL:
                Config::PolygonMode = GL_LINE;
                AppendConsole("\nPolygon mode: GL_LINE.\n");
                break;
            case GL_LINE:
                Config::PolygonMode = GL_FILL;
                AppendConsole("\nPolygon mode: GL_FILL.\n");
                break;
        }
        return true;
    }
    if (cmd == "clrfps" || cmd == "zero_fps_counters")
    {
        maxFPS = 0;
        meanFPS = 50.0f;
        minFPS = xDWORD_MAX;
        return true;
    }
    if (cmd.substr(0, 4) == "log ")
    {
        DEB__log(true, cmd.substr(4).data());
        return true;
    }
    if (cmd == "tail" || cmd == "log_tail")
    {
        char *res = DEB__log_tail();
        AppendConsole("\nLog tail:\n");
        AppendConsole(res);
        return true;
    }
    if (cmd == "read" || cmd == "log_read")
    {
        char *res = DEB__log_read();
        AppendConsole("\nLog file:\n");
        AppendConsole(res);
        return true;
    }
    if (cmd == "clrlog" || cmd == "log_clear")
    {
        DEB__log_clear();
        return true;
    }
    return false;
}

extern int testsLevel0;
extern int testsLevel1;
extern int testsLevel2;
extern int testsLevel3;
extern float time1b;
extern float time2b;

bool SceneConsole::Render()
{
    prevScene->Render();

    GLint cHeight = Height/2;

    glViewport(Left, Top+cHeight, Width, cHeight); // Set viewport
    glDisable(GL_DEPTH_TEST);                      // Disable depth testing
    GLShader::EnableLighting(-1);
    GLShader::EnableTexturing(0);
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

    if (!g_FontMgr.IsHandleValid(font))
        font = g_FontMgr.GetFont("Courier New", 12);
    const GLFont* pFont = g_FontMgr.GetFont(font);

    float lineHeight = pFont->LineH();

    // Draw backgroud
    glColor4f( 0.0f, 0.0f, 0.0f, 0.5f );
    glBegin(GL_QUADS);
        glVertex2f(0.0f, cHeight-1.5f*lineHeight);
        glVertex2f((GLfloat)Width, cHeight-1.5f*lineHeight);
        glVertex2f((GLfloat)Width, (GLfloat)cHeight);
        glVertex2f(0.0f, (GLfloat)cHeight);

        glVertex2f(0.0f, 0.0f);
        glVertex2f((GLfloat)Width, 0.0f);
        glVertex2f((GLfloat)Width, cHeight-2*lineHeight);
        glVertex2f(0.0f, cHeight-2*lineHeight);
    glEnd();

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    pFont->PrintF(0.0f, (float)cHeight-lineHeight, 0.0f,
        "Console    MinFPS: %u MeanFPS: %2u MaxFPS: %u FPS: %2u L0: %5u L1: %5u L2: %5u L3: %5u, T1: %f, T2: %f",
        (int)minFPS, (int) meanFPS, (int)maxFPS, (int)curFPS, testsLevel0,testsLevel1,testsLevel2,testsLevel3, time1b, time2b);
    pFont->PrintF(0.0f, (float)cHeight-2*lineHeight, 0.0f,
        "   Num culled elements: %u", (int)Performance.CulledElements);

    glScissor(0, cHeight, Width, cHeight);                 // Define Scissor Region
    glEnable(GL_SCISSOR_TEST);                             // Enable Scissor Testing

    pFont->Print(0.0f, cHeight-3*lineHeight, 0.0f, cHeight-3*lineHeight, scroll_v, history.data());
    pFont->Print(currCmd.data());
    if (carretVisible && scroll_v >= histLines -1-pageSize)
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

