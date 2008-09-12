// included in MenuStates.h

#include "../../Graphics/OGL/Button.h"
#include "../SceneSkeleton.h"

namespace Scenes { namespace Menu {

    class EditModelState : public BaseState {

        std::vector<Graphics::OGL::Button>  Buttons;
        Filesystem::Vec_string Directories;
        Filesystem::Vec_string Files;
        std::string            CurrentDirectory;

        std::string            ModelGrFile;
        std::string            ModelPhFile;

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Model editor";

            ModelGrFile.clear();
            ModelPhFile.clear();

            CurrentDirectory = Filesystem::GetFullPath("Data/models");
            Buttons.clear();
            GetFiles();
        }

        virtual void Enter()
        {
            ModelGrFile.clear();
            ModelPhFile.clear();
        }

        void GetFiles()
        {
            Directories = Filesystem::GetDirectories(CurrentDirectory);
            Files       = Filesystem::GetFiles(CurrentDirectory, "*.3dx");
            const Filesystem::Vec_string &f3ds = Filesystem::GetFiles(CurrentDirectory, "*.3ds");
            Files.insert(Files.end(), f3ds.begin(), f3ds.end());
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (!BaseState::Update(T_time))
            {
                if (g_InputMgr.InputDown_GetAndRaise(IC_LClick))
                {
                    xFLOAT mouseX = (xFLOAT)g_InputMgr.mouseX;
                    xFLOAT mouseY = (xFLOAT)g_InputMgr.mouseY;

                    for (size_t i = 0; i < Buttons.size(); ++i)
                    {
                        Graphics::OGL::Button &button = Buttons[i];
                        if (button.HitTest(mouseX, mouseY))
                        {
                            if (button.Action == IC_BE_Move)
                            {
                                if (!strcmp(button.Text, ".."))
                                    CurrentDirectory = Filesystem::GetParentDir(CurrentDirectory);
                                else
                                {
                                    CurrentDirectory += "/";
                                    CurrentDirectory += button.Text;
                                }
                                GetFiles();
                            }
                            else
                            if (button.Action == IC_BE_Select)
                            {
                                if (ModelGrFile.size())
                                {
                                    ModelPhFile = CurrentDirectory + "/" + button.Text;

                                    if (ModelPhFile != ModelGrFile)
                                        g_Application.Scene_Set(*
                                        new SceneSkeleton(ModelGrFile.c_str(), ModelPhFile.c_str()), false);
                                    else
                                        g_Application.Scene_Set(*
                                          new SceneSkeleton(ModelGrFile.c_str(), NULL), false);

                                    ModelGrFile.clear();
                                    ModelPhFile.clear();
                                    return true;
                                }
                                else
                                    ModelGrFile = CurrentDirectory + "/" + button.Text;
                            }
                            return true;
                        }
                    }
                }
            }
            return true;
        }

        virtual void Render(const Graphics::OGL::Font* pFont03, const Graphics::OGL::Font* pFont04,
                            const Graphics::OGL::Font* pFont05, const Graphics::OGL::Font* pFont10,
                            xDWORD Width, xDWORD Height)
        {
            xFLOAT lineHeight03 = pFont03->LineH();

            xFLOAT top  = lineHeight03-3, left = 5, width = Width*0.333f-10;
            Buttons.clear();
            for (size_t i=0; i<Directories.size(); ++i)
            {
                top += lineHeight03+3;
                if (top > Height-lineHeight03) { top = lineHeight03*2.f; left += Width*0.333f; }
                Buttons.push_back(Graphics::OGL::Button(Directories[i].c_str(), left, top, width, lineHeight03*1.f, IC_BE_Move));
            }
            for (size_t i=0; i<Files.size(); ++i)
            {
                top += lineHeight03+3;
                if (top > Height-lineHeight03) { top = lineHeight03*2.f; left += Width*0.333f; }
                Buttons.push_back(Graphics::OGL::Button(Files[i].c_str(), left, top, width, lineHeight03*1.f, IC_BE_Select));
                Buttons.back().Background.init(0.35f,0.35f,0.35f,1.f);
            }

            int X = g_InputMgr.mouseX;
            int Y = g_InputMgr.mouseY;

            std::vector<Graphics::OGL::Button>::iterator begin = Buttons.begin();
            std::vector<Graphics::OGL::Button>::iterator end   = Buttons.end();
            for (; begin != end; ++begin)
            {
                begin->Hover((xFLOAT)X, (xFLOAT)Y);
                begin->Down = ModelGrFile == CurrentDirectory + "/" + begin->Text;
                begin->Render(pFont03);
            }
        }
    };

} } // namespace Scenes::Menu
