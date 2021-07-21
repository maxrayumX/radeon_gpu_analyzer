//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <sstream>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "AMDTBaseTools/Include/gtAssert.h"
#include "AMDTOSWrappers/Include/osDirectory.h"
#include "AMDTOSWrappers/Include/osProcess.h"
#include "AMDTOSWrappers/Include/osThread.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder_opengl.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_backend.h"

// Device info.
#include "DeviceInfoUtils.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// The list of devices not supported by VirtualContext.
static const std::set<std::string> kOpenglDisabledDevices = {};

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Internally-linked utilities.
static bool GetVirtualContextPath(std::string& virtual_context_path)
{
#ifdef __linux
    virtual_context_path = "VirtualContext";
#elif _WIN64
    virtual_context_path = "utils\\VirtualContext.exe";
#else
    virtual_context_path = "x86\\VirtualContext.exe";
#endif
    return true;
}

BeProgramBuilderOpengl::BeProgramBuilderOpengl()
{
}

BeProgramBuilderOpengl::~BeProgramBuilderOpengl()
{
}

beKA::beStatus BeProgramBuilderOpengl::GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    (void)table;
    return beKA::kBeStatusInvalid;
}

// Checks if the required output files are generated by the amdspv.
// Only verifies the files requested in the "options.m_pipelineShaders" name list.
static bool  VerifyVirtualContextOutput(const OpenglOptions& options)
{
    bool ret = true;
    if (options.is_amd_isa_disassembly_required)
    {
        ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.compute_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.fragment_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.geometry_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.vertex_shader.asASCIICharArray()));
        assert(ret);
    }
    if (options.is_il_disassembly_required)
    {
        ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.il_disassembly_output_files.compute_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.il_disassembly_output_files.fragment_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.il_disassembly_output_files.geometry_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.il_disassembly_output_files.tessellation_control_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.il_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.il_disassembly_output_files.vertex_shader.asASCIICharArray()));
        assert(ret);
    }
    if (ret && options.is_amd_isa_binaries_required)
    {
        ret &= BeUtils::IsFilePresent(options.program_binary_filename.asASCIICharArray());
        assert(ret);
    }
    if (ret && options.is_stats_required)
    {
        ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.compute_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.fragment_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.geometry_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.tessellation_control_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray()));
        assert(ret);
        ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.vertex_shader.asASCIICharArray()));
        assert(ret);
    }

    return ret;
}

beKA::beStatus BeProgramBuilderOpengl::Compile(const OpenglOptions& gl_options, bool& cancel_signal, bool should_print_cmd, gtString& virtual_context_output)
{
    GT_UNREFERENCED_PARAMETER(cancel_signal);
    beKA::beStatus ret = beKA::kBeStatusSuccess;

    // Clear the output buffer if needed.
    if (!virtual_context_output.isEmpty())
    {
        virtual_context_output.makeEmpty();
    }

    // Get VC's path.
    std::string vcPath;
    GetVirtualContextPath(vcPath);

    AMDTDeviceInfoUtils* pDeviceInfo = AMDTDeviceInfoUtils::Instance();
    if (pDeviceInfo != nullptr)
    {
        const char VC_CMD_DELIMITER = ';';

        // Build the command for invoking Virtual Context.
        std::stringstream cmd;

        // ISA.
        cmd << vcPath << " \"" << gl_options.isa_disassembly_output_files.vertex_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.isa_disassembly_output_files.geometry_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.isa_disassembly_output_files.fragment_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.isa_disassembly_output_files.compute_shader.asASCIICharArray() << VC_CMD_DELIMITER;

        // Program binary.
        cmd << gl_options.program_binary_filename.asASCIICharArray() << VC_CMD_DELIMITER;

        // Statistics.
        cmd << gl_options.stats_output_files.vertex_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.stats_output_files.tessellation_control_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.stats_output_files.geometry_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.stats_output_files.fragment_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.stats_output_files.compute_shader.asASCIICharArray() << VC_CMD_DELIMITER;

        // Target device info.
        cmd << gl_options.chip_family << VC_CMD_DELIMITER << gl_options.chip_revision << VC_CMD_DELIMITER;

        // Input shaders.
        cmd << gl_options.pipeline_shaders.vertex_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.pipeline_shaders.tessellation_control_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.pipeline_shaders.tessellation_evaluation_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.pipeline_shaders.geometry_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.pipeline_shaders.fragment_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.pipeline_shaders.compute_shader.asASCIICharArray() << VC_CMD_DELIMITER;

        // An additional delimiter for the version slot.
        cmd << VC_CMD_DELIMITER;

        // IL disassembly output.
        cmd << gl_options.il_disassembly_output_files.vertex_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.il_disassembly_output_files.tessellation_control_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.il_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.il_disassembly_output_files.geometry_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.il_disassembly_output_files.fragment_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << gl_options.il_disassembly_output_files.compute_shader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << "\"";

        // Build the GL program.
        bool is_compiler_output_relevant = false;
        BeUtils::PrintCmdLine(cmd.str(), should_print_cmd);

        // Workaround for random VirtualContext failures: make 3 attempts with increasing intervals.
        static const unsigned long kVcWaitInternal1 = 2000;
        static const unsigned long kVcWaitInternal2 = 4000;
        bool is_launch_success = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, virtual_context_output);
        if (!is_launch_success || virtual_context_output.isEmpty())
        {
            // First attempt failed, wait and make a second attempt.
            osSleep(kVcWaitInternal1);
            is_launch_success = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, virtual_context_output);

            // Second attempt failed, wait and make the last attempt.
            if (!is_launch_success || virtual_context_output.isEmpty())
            {
                osSleep(kVcWaitInternal2);
                is_launch_success = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, virtual_context_output);
            }
        }

        assert(is_launch_success && !virtual_context_output.isEmpty());
        if (is_launch_success)
        {
            const gtString kVcErrorToken = L"error:";
            gtString vc_output_lower_case = virtual_context_output;
            vc_output_lower_case.toLowerCase();
            if (vc_output_lower_case.find(kVcErrorToken) != -1)
            {
                ret = beKA::kBeStatusOpenglBuildError;
                is_compiler_output_relevant = true;
            }
            else if (!VerifyVirtualContextOutput(gl_options))
            {
                ret = beKA::kBeStatusFailedOutputVerification;
            }
        }
        else
        {
            ret = beKA::kBeStatusOpenglVirtualContextLaunchFailed;
        }

        // Clear the output if irrelevant.
        if (!is_compiler_output_relevant)
        {
            virtual_context_output.makeEmpty();
        }
    }

    return ret;
}

bool BeProgramBuilderOpengl::GetOpenGLVersion(bool should_print_cmd, gtString& opengl_version) const
{
    // Get VC's path.
    std::string vc_path;
    GetVirtualContextPath(vc_path);

    // Build the command for invoking Virtual Context.
    std::stringstream cmd;
    cmd << vc_path << " \";;;;;;;;;;;;;;;;;;;;;version;;;;;;;\"";

    // A flag for canceling the operation, we will not use it.
    bool dummy_cancel_flag = false;
    BeUtils::PrintCmdLine(cmd.str(), should_print_cmd);
    bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), dummy_cancel_flag, opengl_version);

    return isLaunchSuccess;
}

bool BeProgramBuilderOpengl::GetDeviceGLInfo(const std::string& device_name, size_t& device_family_id, size_t& device_revision) const
{
    bool ret = false;

    // This map will hold the device values as expected by the OpenGL backend.
    static std::map<std::string, std::pair<size_t, size_t>> gl_backend_values;
    if (gl_backend_values.empty())
    {
        // Fill in the values if that's the first time.
        gl_backend_values["Bonaire"] = std::pair<int, int>(120, 20);
        gl_backend_values["Bristol Ridge"] = std::pair<int, int>(130, 10);
        gl_backend_values["Capeverde"] = std::pair<int, int>(110, 40);
        gl_backend_values["Carrizo"] = std::pair<int, int>(130, 1);
        gl_backend_values["Fiji"] = std::pair<int, int>(130, 60);
        gl_backend_values["Hainan"] = std::pair<int, int>(110, 75);
        gl_backend_values["Hawaii"] = std::pair<int, int>(120, 40);
        gl_backend_values["Iceland"] = std::pair<int, int>(130, 19);
        gl_backend_values["Kalindi"] = std::pair<int, int>(120, 129);
        gl_backend_values["Mullins"] = std::pair<int, int>(120, 161);
        gl_backend_values["Oland"] = std::pair<int, int>(110, 60);
        gl_backend_values["Pitcairn"] = std::pair<int, int>(110, 20);
        gl_backend_values["Spectre"] = std::pair<int, int>(120, 1);
        gl_backend_values["Spooky"] = std::pair<int, int>(120, 65);
        gl_backend_values["Stoney"] = std::pair<int, int>(130, 97);
        gl_backend_values["Tahiti"] = std::pair<int, int>(110, 0);
        gl_backend_values["Tonga"] = std::pair<int, int>(130, 20);
        gl_backend_values["Baffin"] = std::pair<int, int>(130, 91);
        gl_backend_values["Ellesmere"] = std::pair<int, int>(130, 89);
        gl_backend_values["gfx804"] = std::pair<int, int>(130, 100);
        gl_backend_values["gfx900"] = std::pair<int, int>(141, 1);
        gl_backend_values["gfx902"] = std::pair<int, int>(141, 27);
        gl_backend_values["gfx906"] = std::pair<int, int>(141, 40);
        gl_backend_values["gfx909"] = std::pair<int, int>(141, 20);
        gl_backend_values["gfx90c"] = std::pair<int, int>(141, 20);
        gl_backend_values["gfx1010"] = std::pair<int, int>(143, 1);
        gl_backend_values["gfx1011"] = std::pair<int, int>(143, 10);
        gl_backend_values["gfx1012"] = std::pair<int, int>(143, 20);
        gl_backend_values["gfx1030"] = std::pair<int, int>(143, 40);
        gl_backend_values["gfx1031"] = std::pair<int, int>(143, 50);
        gl_backend_values["gfx1032"] = std::pair<int, int>(143, 60);
        gl_backend_values["gfx1034"] = std::pair<int, int>(143, 70);
    }

    // Fetch the relevant value.
    auto device_iter = gl_backend_values.find(device_name);
    if (device_iter != gl_backend_values.end())
    {
        device_family_id = device_iter->second.first;
        device_revision = device_iter->second.second;
        ret = true;
    }

    return ret;
}

bool BeProgramBuilderOpengl::GetSupportedDevices(std::set<std::string>& device_list)
{
    std::vector<GDT_GfxCardInfo> tmp_card_list;
    bool ret = BeUtils::GetAllGraphicsCards(tmp_card_list, device_list);

    // Remove unsupported devices.
    if (ret)
    {
        for (const std::string& device : kOpenglDisabledDevices)
        {
            device_list.erase(device);
        }
    }
    return ret;
}

const std::set<std::string>& BeProgramBuilderOpengl::GetDisabledDevices()
{
    return kOpenglDisabledDevices;
}
