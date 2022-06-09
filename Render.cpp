#include "Render.hpp"

namespace Spice
{

    Camera* Render::MainCamera = nullptr;
    Camera* Render::EditorCamera = nullptr;

    Shader* Render::DefaultShader = nullptr;
    Shader* Render::TextureShader = nullptr;

    void Render::DrawShape(Camera _camera, Shape& _shape, Shader* _shader, glm::vec2 _pos, float _rotation, glm::vec2 _scale, bool _filled, Spice::Color _color, float _lineWidth)
    {
        _shape.GetBuffer()->Bind();
        glLineWidth(_lineWidth);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0);

        glm::mat4 trans = glm::translate(glm::mat4(1), glm::vec3(_pos.x, _pos.y, 0.0f));
        glm::mat4 rotation = glm::rotate(glm::mat4(1), _rotation, glm::vec3(0, 0, 1));
        glm::mat4 scale = glm::scale(glm::mat4(1), { _scale.x, _scale.y, 1 });

        glm::mat4 model = trans * rotation * scale;
        glm::mat4 mvp = _camera.GetProjMatrix() * _camera.GetViewMatrix() * model;

        _shader->Bind();
        _shader->SetUniform4f("u_Color", _color.r,_color.g, _color.b, _color.a);
        _shader->SetUniformMat4f("u_MVP", mvp);

        GLenum mode = (_filled && _shape.GetPoints().size() > 2) ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
        glDrawArrays(mode, 0, _shape.GetPoints().size());
    }

    void Render::DrawSprite(Camera _camera, Texture& _texture, Shader* _shader, glm::vec2 _pos, glm::vec2 _scale, Spice::Color _modulation)
    {
        _texture.Bind();
        _shader->Bind();
        _shader->SetUniform1i("tex", 0);
        _shader->SetUniform4f("u_Color", 1, 1, 1, 1);

        glm::vec2 scale = glm::vec2(_texture.GetWidth(), _texture.GetHeight()) * _scale;

        float data[]
        {
            0.0f, 0.0f, 0.0f, 0.0f,
            scale.x, 0.0f, 1.0f, 0.0f,
            scale.x, scale.y, 1.0f, 1.0f,
            0.0f, scale.y, 0.0f, 1.0f
        };

        VertexBuffer vbo(data, 4 * 4 * sizeof(float));

        glm::vec2 trans = scale / 2.0f;
        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(_pos.x - trans.x, _pos.y - trans.y, 0));
        
        glm::mat4 mvp = _camera.GetProjMatrix() * _camera.GetViewMatrix() * model;
        _shader->SetUniformMat4f("u_MVP", mvp);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (const void*)(2 * sizeof(float)));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void Render::DrawString(Camera _camera, std::string _str, Shader* _shader, glm::vec2 _pos, glm::vec2 _scale, Spice::Color _modulation)
    {
        _scale *= 4;

        _shader->Bind();
        _shader->SetUniform1i("tex", 1);
        _shader->SetUniform4f("u_Color", _modulation.r, _modulation.g, _modulation.b, _modulation.a);

        // Currently only works with one font, these are the settings for that font.
        int gWidth = 5;
        int gHeight = 7;

        float gWidthScale = (float)gWidth * _scale.x;
        float gHeightScale = (float)gHeight * _scale.x;

        // gS = glyphSize. just getting the normalized dimensions of the characters.
        float gapSizeX = 2.0f / 124.0f;
        float gapSizeY = 2.0f / 52.0f;
        float gSX = 5.0f / 124.0f;
        float gSY = 7.0f / 52.0f;

        int stringSize = _str.size();
        float stringLength = (gWidthScale * stringSize) + ((2 * _scale.x) * stringSize) - (2 * _scale.x);

        Texture fontBitmap((const stbi_uc*) FontBytes, FontBytes_Length);
        fontBitmap.Bind(1);

        for (unsigned int i = 0; i < _str.size(); i++)
        {
            // 18 columns
            // 6 rows

            // character clamped to the bitmap's range
            int c = ((int)_str[i] - (int)32);
            int x = c % 18;
            int y = 5 - c / 18;

            // the exact position of the character in normalized coordinates
            float gX = (x * gSX) + (float)(gapSizeX * x);
            float gY = (y * gSY) + (float)(gapSizeY * y);

            float data[] =
            {
                0.0f, 0.0f, gX, gY,
                gWidthScale, 0.0f, gX + gSX, gY,
                gWidthScale, gHeightScale, gX + gSX, gY + gSY,
                0.0f, gHeightScale, gX, gY + gSY
            };

            VertexBuffer vb(data, 4 * 4 * sizeof(float));
            glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(_pos.x + (i * gWidthScale) + (i * (2 * _scale.x)) - (stringLength / 2), _pos.y - (gHeightScale / 2), 0));
           
            glm::mat4 mvp = _camera.GetProjMatrix() * _camera.GetViewMatrix() * model;
            _shader->SetUniformMat4f("u_MVP", mvp);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), 0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (const void*)(2 * sizeof(float)));

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        }
    }

    void Render::DrawLine(Camera _camera, glm::vec2 _p1, glm::vec2 _p2, Shader* _shader, Spice::Color _modulation, float _lw)
    {
        Shape line({ _p1, _p2 });
        DrawShape(_camera, line, _shader, { 0, 0 }, 0.0f, { 1.0f, 1.0f }, false, _modulation, _lw);
    }

    void Render::Setup(int _w, int _h)
    {
        const std::string defaultVertexSource =
        #include "Vertex.shader"
            ;

        const std::string defaultFragmentSource =
        #include "Fragment.shader"
            ;

        const std::string textureVertexSource =
        #include "TextVertex.shader"
            ;

        const std::string textureFragmentSource =
        #include "TextFragment.shader"
            ;
        
        DefaultShader = new Shader(defaultVertexSource, defaultFragmentSource);
        TextureShader = new Shader(textureVertexSource, textureFragmentSource);

        MainCamera = new Camera(_w, _h);
        EditorCamera = new Camera(_w, _h);
    }

    void Render::ResetViewport(int _w, int _h)
    {
        MainCamera->ResetViewport(_w, _h);
        EditorCamera->ResetViewport(_w, _h);
        glViewport(0, 0, _w, _h);
    }

}
