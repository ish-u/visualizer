#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <SDL.h>
#include <glad/gl.h>

#define CHANNELS 2
#define FREQUENCY 48000
#define SAMPLES 4096
#define PCM_SAMPLE_SIZE SAMPLES * 2
#define PCM_MONO_SAMPLE_SIZE PCM_SAMPLE_SIZE / 2
#define FFT_SAMPLE_SIZE PCM_MONO_SAMPLE_SIZE / 2
#define MAX_PROGRAM_COUNT 10

float pcmSamples[PCM_SAMPLE_SIZE];
float pcmSampleCount = 0;
GLuint pcmSamplesUniformLocation = 0;
GLuint pcmSampleCountUniformLocation = 0;
GLuint pcmSampleTexture;

float fftSamples[FFT_SAMPLE_SIZE];
float fftSampleCount = 0;
GLuint fftSamplesUniformLocation = 0;
GLuint fftSampleCountUniformLocation = 0;
GLuint fftSampleTexture;

GLuint timeUniformLocation = 0;
GLuint resolutionUniformLocation = 0;
GLuint pcmSamplesTextureUniformLocation = 0;
GLuint fftSamplesTextureUniformLocation = 0;

int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

int programCount = 0;
GLuint programs[MAX_PROGRAM_COUNT] = {};

char *readShaderFile(const char *fileName)
{
    // Reading  Shader File in Binary Mode
    FILE *shaderFile = fopen(fileName, "rb");
    if (!shaderFile)
    {
        printf("Failed to read file: %s\n", fileName);
        return NULL;
    }

    // Getting file size
    fseek(shaderFile, 0, SEEK_END);
    long fileSize = ftell(shaderFile);
    rewind(shaderFile);

    // Allocate Buffer (fileSize + 1 -> for null terminator)
    char *buffer = malloc(fileSize + 1);
    if (!buffer)
    {
        printf("Failed to malloc");
        fclose(shaderFile);
        return NULL;
    }

    // Read file into buffer
    fread(buffer, 1, fileSize, shaderFile);
    buffer[fileSize] = '\0';

    fclose(shaderFile);
    return buffer;
}
// Ref -
//  - https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm
//  - https://www.math.wustl.edu/~victor/mfmm/fourier/fft.c
//  - https://rosettacode.org/wiki/Fast_Fourier_transform#C
void ditfft2(complex float *fft, int N)
{
    if (N == 1)
    {
        return;
    }

    complex float *evenFFT =
        malloc(sizeof(complex float) * N / 2);
    complex float *oddFFT =
        malloc(sizeof(complex float) * N / 2);
    for (int i = 0; i < N / 2; i++)
    {
        evenFFT[i] = fft[2 * i];
        oddFFT[i] = fft[2 * i + 1];
    }
    ditfft2(evenFFT, N / 2);
    ditfft2(oddFFT, N / 2);

    for (int i = 0; i < N / 2; i++)
    {
        complex float p = evenFFT[i];
        complex float q = cexp(-2.0 * M_PI * I * i / N) * oddFFT[i];
        fft[i] = p + q;
        fft[i + N / 2] = p - q;
    }

    free(evenFFT);
    free(oddFFT);
}

float *getFFTSamples(float *pcm, int sampleCount)
{
    // Get Mono PCM Data
    int monoSampleCount = sampleCount / 2;
    float *pcmMono = malloc(sizeof(float) * monoSampleCount);
    for (int i = 0; i < monoSampleCount; i++)
    {
        float left = pcm[2 * i];
        float right = pcm[2 * i + 1];
        pcmMono[i] = 0.5f * (left + right);
    }

    // Get FFT from Mono PCM Data
    complex float *fft = malloc(sizeof(complex float) * monoSampleCount);
    for (int i = 0; i < monoSampleCount; i++)
    {
        // Applying Han Window -
        float w = 0.5f * (1 - cosf(2 * M_PI * i / (monoSampleCount - 1)));
        fft[i] = pcmMono[i] * w + I * 0.0;
    }
    ditfft2(fft, monoSampleCount);

    // Convert FFT Data to Magnitude - Only half of the sample count is useful
    float *fftMag = malloc(sizeof(float) * (monoSampleCount / 2));
    for (int i = 0; i < monoSampleCount / 2; i++)
    {
        // Logaritimic Scale - Better for Visuals
        fftMag[i] = 20. * log10f(cabsf(fft[i]) + 1e-6f);
        // maps [-120,0] → [0,1]
        fftMag[i] = (fftMag[i] + 120.0f) / 120.0f;
    }

    free(fft);
    free(pcmMono);

    return fftMag;
}

void audioCaptureCallback(void *userdata, Uint8 *stream, int len)
{
    float *samples = (float *)stream;
    int sampleCount = len / sizeof(float);
    memcpy(pcmSamples, samples, sizeof(float) * PCM_SAMPLE_SIZE);

    float *getFFTSamplesResult = getFFTSamples(samples, sampleCount);
    memcpy(fftSamples, getFFTSamplesResult, sizeof(float) * FFT_SAMPLE_SIZE);
    free(getFFTSamplesResult);
};

GLuint createGraphicsProgram(const char *fragmentShaderSourcePath)
{
    const char *vertexShaderSource = readShaderFile("./shaders/vert.vs");
    const char *fragmentShaderSource = readShaderFile(fragmentShaderSourcePath);
    if (fragmentShaderSource == NULL)
    {
        return -1;
    }

    GLuint graphicsPipelineShaderProgram = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint vertex_compiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertexShader, 1024, &log_length, message);
        printf("Failed to compile vertex shader: %s", message);
        return 1;
    }

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLint fragment_compiled;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragmentShader, 1024, &log_length, message);
        printf("Failed to compile fragment shader: %s", message);
        return 1;
    }

    glAttachShader(graphicsPipelineShaderProgram, vertexShader);
    glAttachShader(graphicsPipelineShaderProgram, fragmentShader);
    glLinkProgram(graphicsPipelineShaderProgram);
    GLint program_linked;
    glGetProgramiv(graphicsPipelineShaderProgram, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(graphicsPipelineShaderProgram, 1024, &log_length, message);
        printf("Failed to link shader program : %s", message);
        return 1;
    }

    glValidateProgram(graphicsPipelineShaderProgram);

    glDetachShader(graphicsPipelineShaderProgram, vertexShader);
    glDetachShader(graphicsPipelineShaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free((void *)vertexShaderSource);
    free((void *)fragmentShaderSource);

    if (programCount < MAX_PROGRAM_COUNT)
    {
        programs[programCount++] = graphicsPipelineShaderProgram;
        return graphicsPipelineShaderProgram;
    }

    return -1;
}

GLuint useGraphicsProgram(int index)
{
    if (index < programCount)
    {
        GLuint graphicsPipelineShaderProgram = programs[index];

        // Setup
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glUseProgram(graphicsPipelineShaderProgram);

        // Uniforms
        timeUniformLocation = glGetUniformLocation(graphicsPipelineShaderProgram, "iTime");
        if (timeUniformLocation == -1)
        {
            printf("Failed to get uniform 'iTime'\n");
        }
        resolutionUniformLocation = glGetUniformLocation(graphicsPipelineShaderProgram, "iResolution");
        if (resolutionUniformLocation == -1)
        {
            printf("Failed to get uniform 'iResolution'\n");
        }
        glUniform3f(resolutionUniformLocation, WINDOW_WIDTH, WINDOW_HEIGHT, 1);
        pcmSamplesTextureUniformLocation = glGetUniformLocation(graphicsPipelineShaderProgram, "pcmSampleTexture");
        if (pcmSamplesTextureUniformLocation == -1)
        {
            printf("Failed to get uniform 'pcmSampleTexture'\n");
        }
        glUniform1i(pcmSamplesTextureUniformLocation, 0);
        fftSamplesTextureUniformLocation = glGetUniformLocation(graphicsPipelineShaderProgram, "fftSampleTexture");
        if (fftSamplesTextureUniformLocation == -1)
        {
            printf("Failed to get uniform 'fftSampleTexture'\n");
        }
        glUniform1i(fftSamplesTextureUniformLocation, 1);

        return graphicsPipelineShaderProgram;
    }

    return -1;
}

int main(int argc, char *argv[])
{
    // Pointer to SDL Window Struct
    SDL_Window *window = NULL;
    SDL_GLContext openGLContext = NULL;

    // Initializing SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Error initialzing SDL: %s", SDL_GetError());
        return 1;
    }

    int recordingDeviceCount = SDL_GetNumAudioDevices(SDL_TRUE);
    if (recordingDeviceCount < 1)
    {
        printf("Unable to get audio capture device! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    for (int i = 0; i < recordingDeviceCount; ++i)
    {
        const char *deviceName = SDL_GetAudioDeviceName(i, SDL_TRUE);
        printf("%d - %s\n", i, deviceName);
    }

    // Audio Capture
    SDL_AudioDeviceID captureDeviceId = 0;
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = FREQUENCY;
    desiredSpec.format = AUDIO_F32;
    desiredSpec.channels = CHANNELS;
    desiredSpec.samples = SAMPLES;
    desiredSpec.callback = audioCaptureCallback;

    SDL_AudioSpec obtainedSpec;
    SDL_zero(obtainedSpec);

    // Hardcoding 1 - BlackHole 2ch
    captureDeviceId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(1, SDL_TRUE), SDL_TRUE, &desiredSpec, &obtainedSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (captureDeviceId == 0)
    {
        printf("Failed to open capture device : %s", SDL_GetError());
        return 1;
    }

    // Get Screen Resolution
    SDL_DisplayMode displayMode;
    if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
    {
        printf("SDL_GetDesktopDisplayMode failed: %s\n", SDL_GetError());
        return 1;
    }

    WINDOW_WIDTH = displayMode.w;
    WINDOW_HEIGHT = displayMode.h;

    // Setting OpenGL Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Creating Window
    window = SDL_CreateWindow("Visualizer", SDL_WINDOWPOS_UNDEFINED_DISPLAY(0), SDL_WINDOWPOS_UNDEFINED_DISPLAY(1), WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
    if (!window)
    {
        printf("Error creating window: %s", SDL_GetError());
        return 1;
    }

    // Creating OpenGL Context
    openGLContext = SDL_GL_CreateContext(window);
    if (!openGLContext)
    {
        printf("Error creating OpenGL Context: %s", SDL_GetError());
        return 1;
    }

    // Initialize GLAD
    if (!gladLoaderLoadGL())
    {
        printf("Failed to initialize GLAD: %s", SDL_GetError());
        return 1;
    }

    // OpenGL Info
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Vertex
    GLfloat vertexPosition[9] = {
        -1.0f,
        -1.0f,
        0.0f,
        3.0f,
        -1.0f,
        0.0f,
        -1.0f,
        3.0f,
        0.0f,
    };

    // VAO
    GLuint vertexArrayObject = 0;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // VBO
    GLuint vertexBufferObject = 0;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition), &vertexPosition, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Clean up
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);

    // PCM Sample Texture
    glGenTextures(1, &pcmSampleTexture);
    glBindTexture(GL_TEXTURE_1D, pcmSampleTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, PCM_SAMPLE_SIZE, 0, GL_RED, GL_FLOAT, NULL);

    // PCM Sample Texture
    glGenTextures(1, &fftSampleTexture);
    glBindTexture(GL_TEXTURE_1D, fftSampleTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, FFT_SAMPLE_SIZE, 0, GL_RED, GL_FLOAT, NULL);

    // Graphics Programs
    createGraphicsProgram("./shaders/fft.fs");
    createGraphicsProgram("./shaders/pcm.fs");
    createGraphicsProgram("./shaders/grid.fs");
    createGraphicsProgram("./shaders/squares.fs");
    createGraphicsProgram("./shaders/hearts.fs");

    int currentGraphicsProgramIndex = 0;
    useGraphicsProgram(currentGraphicsProgramIndex);

    // Loop
    int quit = 0;
    SDL_Event event;
    SDL_PauseAudioDevice(captureDeviceId, SDL_FALSE);

    while (!quit)
    {
        float time = SDL_GetTicks() / 1000.0f;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    currentGraphicsProgramIndex = (currentGraphicsProgramIndex - 1) % programCount;
                    useGraphicsProgram(currentGraphicsProgramIndex);
                    break;
                case SDLK_RIGHT:
                    currentGraphicsProgramIndex = (currentGraphicsProgramIndex + 1) % programCount;
                    useGraphicsProgram(currentGraphicsProgramIndex);
                    break;
                case SDLK_q:
                    quit = 1;
                    break;
                default:
                    break;
                }
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);

        // Render
        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glUniform1f(timeUniformLocation, time);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, pcmSampleTexture);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, PCM_SAMPLE_SIZE, GL_RED, GL_FLOAT, pcmSamples);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, fftSampleTexture);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, FFT_SAMPLE_SIZE, GL_RED, GL_FLOAT, fftSamples);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Update window with OpenGL
        SDL_GL_SwapWindow(window);
    }

    // Destroy window
    SDL_PauseAudioDevice(captureDeviceId, SDL_TRUE);
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    return 0;
}
