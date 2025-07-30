/*
 *
 * Copyright (C) 2024-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"

#include "loader/ze_loader.h"
#include "ze_api.h"
#include "zes_api.h"

#include <fstream>

#if defined(_WIN32)
    #include <io.h>
    #include <cstdio>
    #include <fcntl.h>
    #include <windows.h>
    #define putenv_safe _putenv
#else
    #include <cstdlib>
    #include <sys/types.h>
    #include <unistd.h>
    #define _dup dup
    #define _dup2 dup2
    #define _close close
    #define putenv_safe putenv
#endif

namespace {

  inline std::string getenv_string(const char *name)
  {

    const char *env = nullptr;
#if defined(_WIN32)
    char buffer[1024];
    auto rc = GetEnvironmentVariable(name, buffer, 1024);
    if (0 != rc && rc <= 1024)
    {
      env = buffer;
    }
#else
    env = getenv(name);
#endif

    if ((nullptr == env))
      return "";
    return std::string(env);
  }

  bool compare_env(const char *api, std::string value)
  {
    auto val = getenv_string(api);
    if (strcmp(val.c_str(), value.c_str()) == 0)
    {
      return true;
    }
    return false;
  }

TEST(
    LoaderAPI,
    GivenLevelZeroLoaderPresentWhenCallingzeGetLoaderVersionsAPIThenValidVersionIsReturned) {

  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));

  size_t size = 0;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderGetVersions(&size, nullptr));
  EXPECT_GT(size, 0);

  std::vector<zel_component_version_t> versions(size);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderGetVersions(&size, versions.data()));

  std::cout << "Found " << versions.size() << " versions" << std::endl;
  std::cout << std::endl;
  const std::string loader_name = "loader";
  for (auto &component : versions) {
    std::cout << "component.component_name: " << component.component_name << std::endl;
    std::cout << "component.component_lib_version.major: " << component.component_lib_version.major << std::endl;
    std::cout << "component.spec_version: " << component.spec_version << std::endl;
    std::cout << "component.component_lib_name: " << component.component_name << std::endl;
    std::cout << std::endl;

    if (loader_name == component.component_name) {
      EXPECT_GE(component.component_lib_version.major, 1);
    }
  }
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithTypesUnsupportedWithFailureThenSupportedTypesThenSuccessReturned) {

  uint32_t pCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_NPU;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=GPU" ) );
  EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_EQ(pCount, 0);
  pCount = 0;
  desc.flags = UINT32_MAX;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithGPUTypeThenExpectPassWithGPUorAllOnly) {

  uint32_t pCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_GPU;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=GPU" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = UINT32_MAX;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_GPU | ZE_INIT_DRIVER_TYPE_FLAG_NPU;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithNPUTypeThenExpectPassWithNPUorAllOnly) {

  uint32_t pCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_NPU;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=NPU" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = UINT32_MAX;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_GPU | ZE_INIT_DRIVER_TYPE_FLAG_NPU;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithAnyTypeWithNullDriverAcceptingAllThenExpectatLeast1Driver) {

  uint32_t pCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_NPU;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=ALL" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = UINT32_MAX;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_GPU;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
  pCount = 0;
  desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_GPU | ZE_INIT_DRIVER_TYPE_FLAG_NPU;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
  EXPECT_GT(pCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversThenzeInitThenBothCallsSucceedWithAllTypes) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=ALL" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversThenzeInitThenBothCallsSucceedWithGPUTypes) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=GPU" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_GPU_ONLY));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
    LoaderInit,
    GivenZeInitDriversUnsupportedOnTheDriverWhenCallingZeInitDriversThenUninitializedReturned) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_MISSING_API=zeInitDrivers" ) );
  EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_EQ(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingZeInitDriversThenzeInitThenBothCallsSucceedWithNPUTypes) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=NPU" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_VPU_ONLY));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingzeInitThenZeInitDriversThenBothCallsSucceedWithAllTypes) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=ALL" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingzeInitThenZeInitDriversThenBothCallsSucceedWithGPUTypes) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=GPU" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_GPU_ONLY));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
    LoaderInit,
    GivenLevelZeroLoaderPresentWhenCallingzeInitThenZeInitDriversThenBothCallsSucceedWithNPUTypes) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_TYPE=NPU" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_VPU_ONLY));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversMissingInitDriversWhenCallingZeInitDriversThenExpectSuccessForZeInit) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_MISSING_API=zeInitDrivers" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
  EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_EQ(pInitDriversCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversMissingInitDriversInOneDriverWhenCallingZeInitDriversThenExpectSuccessForZeInitDrivers) {

  uint32_t pInitDriversCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_MISSING_API_DRIVER_ID=zeInitDrivers:1" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversMissingInitDriversWhenCallingZeInitDriversThenExpectSuccessForZeInitWithDriverGetAfterInitDrivers) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  putenv_safe( const_cast<char *>( "ZEL_TEST_MISSING_API=zeInitDrivers" ) );
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_EQ(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZeInitDriversThenExpectSuccessForZeInit) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZeInitThenZeInitDriversThenExpectSuccessForZeInitWithDriverGetAfterInitDrivers) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZesInitThenExpectSuccessForZesDriverGet) {

  uint32_t pDriverGetCount = 0;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZesInitThenZeInitDriversExpectSuccessForZesDriverGetAndZeInitDrivers) {

  uint32_t pInitDriversCount = 0;
  uint32_t pDriverGetCount = 0;
  ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
  desc.flags = UINT32_MAX;
  desc.pNext = nullptr;
  EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
  EXPECT_GT(pInitDriversCount, 0);
  EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&pDriverGetCount, nullptr));
  EXPECT_GT(pDriverGetCount, 0);
}

TEST(
  LoaderTearDown,
  GivenLoaderNotInDestructionStateWhenCallingzelCheckIsLoaderInTearDownThenFalseIsReturned) {

  EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
  EXPECT_FALSE(zelCheckIsLoaderInTearDown());
  EXPECT_FALSE(zelCheckIsLoaderInTearDown());
  EXPECT_FALSE(zelCheckIsLoaderInTearDown());
  EXPECT_FALSE(zelCheckIsLoaderInTearDown());
}

class CaptureOutput {
private:
    int original_fd;
    int fd;
    int stream;
    char filename[50] = "/tmp/capture_output_XXXXXX";

public:
    enum { Stdout = 1, Stderr = 2 };

    CaptureOutput(int stream_) : stream(stream_) {
        original_fd = _dup(stream); 
#if defined(__linux__)
        fd = mkstemp(filename);
#elif defined(_WIN32)
        tmpnam_s(filename, sizeof(filename));
        _sopen_s(&fd, filename, _O_CREAT | _O_RDWR, _SH_DENYNO, _S_IREAD | _S_IWRITE);
#endif
        fflush(nullptr);
        _dup2(fd, stream);
        _close(fd);
    }

    ~CaptureOutput() {
        if (original_fd != -1) {
            fflush(nullptr);
            _dup2(original_fd, stream);
            _close(original_fd);
            original_fd = -1;
        }
        if (remove(filename) != 0) {
            std::cerr << "Deleting file " << filename << " failed.";
        }
    }

    std::string GetOutput() {
        if (original_fd != -1) {
            fflush(nullptr);
            _dup2(original_fd, stream);
            _close(original_fd);
            original_fd = -1;
        }
        std::ifstream stream(filename);
        std::string output = std::string((std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());
        return output;
    }
};

TEST(
    LoaderInitDrivers,
    GivenZeInitDriverWhenCalledThenNoOutputIsPrintedToStdout) {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = { ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC };
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;

    CaptureOutput capture(CaptureOutput::Stdout);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    
    std::string output = capture.GetOutput();
    EXPECT_TRUE(output.empty());
}

TEST(
  LoaderInit,
  GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingDriverGetPropertiesThenExpectSuccess) {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
    std::vector<ze_driver_handle_t> drivers(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    for (uint32_t i = 0; i < pDriverGetCount; ++i) {
      ze_driver_properties_t driverProperties = {ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[i], &driverProperties));
      std::cout << "Driver " << i << " properties:" << std::endl;
      std::cout << "  Driver version: " << driverProperties.driverVersion << std::endl;
      std::cout << "  UUID: ";
      for (auto byte : driverProperties.uuid.id) {
        std::cout << std::hex << static_cast<int>(byte);
      }
      std::cout << std::dec << std::endl;
    }
}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForModule) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
EXPECT_GT(pInitDriversCount, 0);
std::vector<ze_driver_handle_t> drivers(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_module_handle_t module;
ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, nullptr));
ze_module_handle_t translatedHandle = module;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE, module, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, module);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForModule) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_module_handle_t module;
ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, nullptr));
ze_module_handle_t translatedHandle = module;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE, module, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, module);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForModuleBuildLog) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_module_handle_t module;
ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
ze_module_build_log_handle_t buildLog;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, &buildLog));
ze_module_build_log_handle_t translatedHandle = buildLog;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE_BUILD_LOG, buildLog, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, buildLog);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleBuildLogDestroy(buildLog));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForModuleBuildLog) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_module_handle_t module;
ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
ze_module_build_log_handle_t buildLog;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, &buildLog));
ze_module_build_log_handle_t translatedHandle = buildLog;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE_BUILD_LOG, buildLog, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, buildLog);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleBuildLogDestroy(buildLog));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForKernel) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_module_handle_t module;
ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, nullptr));
ze_kernel_handle_t kernel;
ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelCreate(module, &kernelDesc, &kernel));
ze_kernel_handle_t translatedHandle = kernel;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_KERNEL, kernel, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, kernel);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForKernel) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_module_handle_t module;
ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, nullptr));
ze_kernel_handle_t kernel;
ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelCreate(module, &kernelDesc, &kernel));
ze_kernel_handle_t translatedHandle = kernel;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_KERNEL, kernel, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, kernel);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForSampler) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_sampler_handle_t sampler;
ze_sampler_desc_t samplerDesc = {ZE_STRUCTURE_TYPE_SAMPLER_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerCreate(context, devices[0], &samplerDesc, &sampler));
ze_sampler_handle_t translatedHandle = sampler;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_SAMPLER, sampler, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, sampler);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerDestroy(sampler));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForSampler) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_sampler_handle_t sampler;
ze_sampler_desc_t samplerDesc = {ZE_STRUCTURE_TYPE_SAMPLER_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerCreate(context, devices[0], &samplerDesc, &sampler));
ze_sampler_handle_t translatedHandle = sampler;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_SAMPLER, sampler, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, sampler);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerDestroy(sampler));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForPhysicalMem) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_physical_mem_handle_t physicalMem;
ze_physical_mem_desc_t physicalMemDesc = {ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemCreate(context, devices[0], &physicalMemDesc, &physicalMem));
ze_physical_mem_handle_t translatedHandle = physicalMem;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_PHYSICAL_MEM, physicalMem, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, physicalMem);
EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemDestroy(context, physicalMem));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForPhysicalMem) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_physical_mem_handle_t physicalMem;
ze_physical_mem_desc_t physicalMemDesc = {ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemCreate(context, devices[0], &physicalMemDesc, &physicalMem));
ze_physical_mem_handle_t translatedHandle = physicalMem;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_PHYSICAL_MEM, physicalMem, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, physicalMem);
EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemDestroy(context, physicalMem));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForFence) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_command_queue_handle_t commandQueue;
ze_command_queue_desc_t commandQueueDesc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC};
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, devices[0], &commandQueueDesc, &commandQueue));
ze_fence_handle_t fence;
ze_fence_desc_t fenceDesc = {ZE_STRUCTURE_TYPE_FENCE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceCreate(commandQueue, &fenceDesc, &fence));
ze_fence_handle_t translatedHandle = fence;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_FENCE, fence, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, fence);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceDestroy(fence));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForFence) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_command_queue_handle_t commandQueue;
ze_command_queue_desc_t commandQueueDesc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC};
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, devices[0], &commandQueueDesc, &commandQueue));
ze_fence_handle_t fence;
ze_fence_desc_t fenceDesc = {ZE_STRUCTURE_TYPE_FENCE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceCreate(commandQueue, &fenceDesc, &fence));
ze_fence_handle_t translatedHandle = fence;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_FENCE, fence, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, fence);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceDestroy(fence));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForEventPool) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_event_pool_handle_t eventPool;
ze_event_pool_desc_t eventPoolDesc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC};
eventPoolDesc.count = 1;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &eventPoolDesc, 0, nullptr, &eventPool));
ze_event_pool_handle_t translatedHandle = eventPool;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT_POOL, eventPool, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, eventPool);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForEventPool) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_event_pool_handle_t eventPool;
ze_event_pool_desc_t eventPoolDesc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC};
eventPoolDesc.count = 1;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &eventPoolDesc, 0, nullptr, &eventPool));
ze_event_pool_handle_t translatedHandle = eventPool;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT_POOL, eventPool, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, eventPool);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForImage) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_image_handle_t image;
ze_image_desc_t imageDesc = {ZE_STRUCTURE_TYPE_IMAGE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageCreate(context, devices[0], &imageDesc, &image));
ze_image_handle_t translatedHandle = image;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_IMAGE, image, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, image);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageDestroy(image));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForImage) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_image_handle_t image;
ze_image_desc_t imageDesc = {ZE_STRUCTURE_TYPE_IMAGE_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageCreate(context, devices[0], &imageDesc, &image));
ze_image_handle_t translatedHandle = image;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_IMAGE, image, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, image);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageDestroy(image));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForContext) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_context_handle_t translatedHandle = context;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_CONTEXT, context, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, context);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForContext) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_context_handle_t translatedHandle = context;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_CONTEXT, context, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, context);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForCommandQueue) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_command_queue_handle_t commandQueue;
ze_command_queue_desc_t commandQueueDesc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC};
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, devices[0], &commandQueueDesc, &commandQueue));
ze_command_queue_handle_t translatedHandle = commandQueue;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_QUEUE, commandQueue, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, commandQueue);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForCommandQueue) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_command_queue_handle_t commandQueue;
ze_command_queue_desc_t commandQueueDesc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC};
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, devices[0], &commandQueueDesc, &commandQueue));
ze_command_queue_handle_t translatedHandle = commandQueue;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_QUEUE, commandQueue, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, commandQueue);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForCommandList) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_command_list_handle_t commandList;
ze_command_list_desc_t commandListDesc = {ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC};
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListCreate(context, devices[0], &commandListDesc, &commandList));
ze_command_list_handle_t translatedHandle = commandList;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_LIST, commandList, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, commandList);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListDestroy(commandList));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForCommandList) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_command_list_handle_t commandList;
ze_command_list_desc_t commandListDesc = {ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC};
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListCreate(context, devices[0], &commandListDesc, &commandList));
ze_command_list_handle_t translatedHandle = commandList;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_LIST, commandList, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, commandList);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListDestroy(commandList));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForEvent) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_event_pool_handle_t eventPool;
ze_event_pool_desc_t eventPoolDesc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC};
eventPoolDesc.count = 1;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &eventPoolDesc, 0, nullptr, &eventPool));
ze_event_handle_t event;
ze_event_desc_t eventDesc = {ZE_STRUCTURE_TYPE_EVENT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventCreate(eventPool, &eventDesc, &event));
ze_event_handle_t translatedHandle = event;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT, event, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, event);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForEvent) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_context_handle_t context;
ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[0], &contextDesc, &context));
ze_event_pool_handle_t eventPool;
ze_event_pool_desc_t eventPoolDesc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC};
eventPoolDesc.count = 1;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &eventPoolDesc, 0, nullptr, &eventPool));
ze_event_handle_t event;
ze_event_desc_t eventDesc = {ZE_STRUCTURE_TYPE_EVENT_DESC};
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventCreate(eventPool, &eventDesc, &event));
ze_event_handle_t translatedHandle = event;
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT, event, reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, event);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForDriver) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_driver_handle_t translatedHandle = drivers[0];
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DRIVER, drivers[0], reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, drivers[0]);

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForDriver) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
ze_driver_handle_t translatedHandle = drivers[0];
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DRIVER, drivers[0], reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, drivers[0]);

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForDevice) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_device_handle_t translatedHandle = devices[0];
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DEVICE, devices[0], reinterpret_cast<void**>(&translatedHandle)));
EXPECT_NE(translatedHandle, devices[0]);

}

TEST(
  LoaderTranslateHandles,
  GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForDevice) {

uint32_t pInitDriversCount = 0;
ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
desc.flags = UINT32_MAX;
desc.pNext = nullptr;
putenv_safe( const_cast<char *>( "ZE_ENABLE_LOADER_INTERCEPT=1" ) );
putenv_safe( const_cast<char *>( "ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0" ) );
std::vector<ze_driver_handle_t> drivers;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
drivers.resize(pInitDriversCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
EXPECT_GT(pInitDriversCount, 0);
ze_driver_ddi_handles_ext_properties_t driverDdiHandlesExtProperties = {};
driverDdiHandlesExtProperties.stype = ZE_STRUCTURE_TYPE_DRIVER_DDI_HANDLES_EXT_PROPERTIES;
driverDdiHandlesExtProperties.pNext = nullptr;
ze_driver_properties_t properties = {};
properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
properties.pNext = &driverDdiHandlesExtProperties;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[0], &properties));
EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
uint32_t deviceCount = 0;
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, nullptr));
std::vector<ze_device_handle_t> devices(deviceCount);
EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[0], &deviceCount, devices.data()));
ze_device_handle_t translatedHandle = devices[0];
EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DEVICE, devices[0], reinterpret_cast<void**>(&translatedHandle)));
EXPECT_EQ(translatedHandle, devices[0]);

}

TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingSysManVfApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_vf_handle_t vfHandle{};
      zes_vf_exp_properties_t vf_properties{};
      zes_vf_util_mem_exp_t util_mem_exp{};
      zes_vf_util_mem_exp2_t util_mem_exp2{};
      zes_vf_util_engine_exp_t util_engine_exp{};
      zes_vf_util_engine_exp2_t util_engine_exp2{};
      zes_vf_exp_capabilities_t vf_exp_capabilities{};
      zes_vf_exp2_capabilities_t vf_exp2_capabilities{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumActiveVFExp(deviceHandle, &count, &vfHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumActiveVFExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFPropertiesExp(vfHandle, &vf_properties));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFPropertiesExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFMemoryUtilizationExp(vfHandle, &count, &util_mem_exp));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFMemoryUtilizationExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFEngineUtilizationExp(vfHandle, &count, &util_engine_exp));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFEngineUtilizationExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementSetVFTelemetryModeExp(vfHandle, 0, 0));
      EXPECT_TRUE(compare_env("zesVFManagementSetVFTelemetryModeExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementSetVFTelemetrySamplingIntervalExp(vfHandle, 0, 0));
      EXPECT_TRUE(compare_env("zesVFManagementSetVFTelemetrySamplingIntervalExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumEnabledVFExp(deviceHandle, &count, &vfHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumEnabledVFExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFCapabilitiesExp(vfHandle, &vf_exp_capabilities));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFCapabilitiesExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFMemoryUtilizationExp2(vfHandle, &count, &util_mem_exp2));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFMemoryUtilizationExp2", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFEngineUtilizationExp2(vfHandle, &count, &util_engine_exp2));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFEngineUtilizationExp2", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesVFManagementGetVFCapabilitiesExp2(vfHandle, &vf_exp2_capabilities));
      EXPECT_TRUE(compare_env("zesVFManagementGetVFCapabilitiesExp2", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingPowerManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_pwr_handle_t powerHandle{};
      zes_power_properties_t powerProperties{};
      zes_power_energy_counter_t energyCounter{};
      zes_energy_threshold_t energyThreshold{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumPowerDomains(deviceHandle, &count, &powerHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumPowerDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerGetProperties(powerHandle, &powerProperties));
      EXPECT_TRUE(compare_env("zesPowerGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerGetEnergyCounter(powerHandle, &energyCounter));
      EXPECT_TRUE(compare_env("zesPowerGetEnergyCounter", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerGetLimitsExt(powerHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesPowerGetLimitsExt", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerSetLimitsExt(powerHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesPowerSetLimitsExt", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerGetEnergyThreshold(powerHandle, &energyThreshold));
      EXPECT_TRUE(compare_env("zesPowerGetEnergyThreshold", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerSetEnergyThreshold(powerHandle, 100.0));
      EXPECT_TRUE(compare_env("zesPowerSetEnergyThreshold", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFrequencyManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_freq_handle_t freqHandle{};
      zes_freq_properties_t freqProperties{};
      zes_freq_range_t freqRange{};
      zes_freq_state_t freqState{};
      zes_freq_throttle_time_t throttleTime{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumFrequencyDomains(deviceHandle, &count, &freqHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumFrequencyDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyGetProperties(freqHandle, &freqProperties));
      EXPECT_TRUE(compare_env("zesFrequencyGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyGetAvailableClocks(freqHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesFrequencyGetAvailableClocks", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyGetRange(freqHandle, &freqRange));
      EXPECT_TRUE(compare_env("zesFrequencyGetRange", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencySetRange(freqHandle, &freqRange));
      EXPECT_TRUE(compare_env("zesFrequencySetRange", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyGetState(freqHandle, &freqState));
      EXPECT_TRUE(compare_env("zesFrequencyGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyGetThrottleTime(freqHandle, &throttleTime));
      EXPECT_TRUE(compare_env("zesFrequencyGetThrottleTime", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingEngineUtilizationApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_engine_handle_t engineHandle{};
      zes_engine_properties_t engineProperties{};
      zes_engine_stats_t engineStats{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumEngineGroups(deviceHandle, &count, &engineHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumEngineGroups", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesEngineGetProperties(engineHandle, &engineProperties));
      EXPECT_TRUE(compare_env("zesEngineGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesEngineGetActivity(engineHandle, &engineStats));
      EXPECT_TRUE(compare_env("zesEngineGetActivity", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesEngineGetActivityExt(engineHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesEngineGetActivityExt", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingStandbyManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_standby_handle_t standbyHandle{};
      zes_standby_properties_t standbyProperties{};
      zes_standby_promo_mode_t standbyMode{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumStandbyDomains(deviceHandle, &count, &standbyHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumStandbyDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesStandbyGetProperties(standbyHandle, &standbyProperties));
      EXPECT_TRUE(compare_env("zesStandbyGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesStandbyGetMode(standbyHandle, &standbyMode));
      EXPECT_TRUE(compare_env("zesStandbyGetMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesStandbySetMode(standbyHandle, ZES_STANDBY_PROMO_MODE_DEFAULT));
      EXPECT_TRUE(compare_env("zesStandbySetMode", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingMemoryManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_mem_handle_t memHandle{};
      zes_mem_properties_t memProperties{};
      zes_mem_state_t memState{};
      zes_mem_bandwidth_t memBandwidth{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumMemoryModules(deviceHandle, &count, &memHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumMemoryModules", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesMemoryGetProperties(memHandle, &memProperties));
      EXPECT_TRUE(compare_env("zesMemoryGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesMemoryGetState(memHandle, &memState));
      EXPECT_TRUE(compare_env("zesMemoryGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesMemoryGetBandwidth(memHandle, &memBandwidth));
      EXPECT_TRUE(compare_env("zesMemoryGetBandwidth", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFabricPortManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_fabric_port_handle_t fabricPortHandle{};
      zes_fabric_port_properties_t fabricPortProperties{};
      zes_fabric_link_type_t linkType{};
      zes_fabric_port_config_t portConfig{};
      zes_fabric_port_state_t portState{};
      zes_fabric_port_throughput_t portThroughput{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumFabricPorts(deviceHandle, &count, &fabricPortHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumFabricPorts", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFabricPortGetProperties(fabricPortHandle, &fabricPortProperties));
      EXPECT_TRUE(compare_env("zesFabricPortGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFabricPortGetLinkType(fabricPortHandle, &linkType));
      EXPECT_TRUE(compare_env("zesFabricPortGetLinkType", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFabricPortGetConfig(fabricPortHandle, &portConfig));
      EXPECT_TRUE(compare_env("zesFabricPortGetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFabricPortSetConfig(fabricPortHandle, &portConfig));
      EXPECT_TRUE(compare_env("zesFabricPortSetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFabricPortGetState(fabricPortHandle, &portState));
      EXPECT_TRUE(compare_env("zesFabricPortGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFabricPortGetThroughput(fabricPortHandle, &portThroughput));
      EXPECT_TRUE(compare_env("zesFabricPortGetThroughput", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingTemperatureMonitoringApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_temp_handle_t tempHandle{};
      zes_temp_properties_t tempProperties{};
      zes_temp_config_t tempConfig{};
      double temperature = 0.0;

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumTemperatureSensors(deviceHandle, &count, &tempHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumTemperatureSensors", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesTemperatureGetProperties(tempHandle, &tempProperties));
      EXPECT_TRUE(compare_env("zesTemperatureGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesTemperatureGetConfig(tempHandle, &tempConfig));
      EXPECT_TRUE(compare_env("zesTemperatureGetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesTemperatureSetConfig(tempHandle, &tempConfig));
      EXPECT_TRUE(compare_env("zesTemperatureSetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesTemperatureGetState(tempHandle, &temperature));
      EXPECT_TRUE(compare_env("zesTemperatureGetState", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingPsuManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_psu_handle_t psuHandle{};
      zes_psu_properties_t psuProperties{};
      zes_psu_state_t psuState{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumPsus(deviceHandle, &count, &psuHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumPsus", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPsuGetProperties(psuHandle, &psuProperties));
      EXPECT_TRUE(compare_env("zesPsuGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPsuGetState(psuHandle, &psuState));
      EXPECT_TRUE(compare_env("zesPsuGetState", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFanManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_fan_handle_t fanHandle{};
      zes_fan_properties_t fanProperties{};
      zes_fan_config_t fanConfig{};
      zes_fan_speed_t fanSpeed{};
      zes_fan_speed_table_t speedTable{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumFans(deviceHandle, &count, &fanHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumFans", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFanGetProperties(fanHandle, &fanProperties));
      EXPECT_TRUE(compare_env("zesFanGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFanGetConfig(fanHandle, &fanConfig));
      EXPECT_TRUE(compare_env("zesFanGetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFanSetDefaultMode(fanHandle));
      EXPECT_TRUE(compare_env("zesFanSetDefaultMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFanSetFixedSpeedMode(fanHandle, &fanSpeed));
      EXPECT_TRUE(compare_env("zesFanSetFixedSpeedMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFanSetSpeedTableMode(fanHandle, &speedTable));
      EXPECT_TRUE(compare_env("zesFanSetSpeedTableMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFanGetState(fanHandle, ZES_FAN_SPEED_UNITS_RPM, &fanSpeed.speed));
      EXPECT_TRUE(compare_env("zesFanGetState", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingLedManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_led_handle_t ledHandle{};
      zes_led_properties_t ledProperties{};
      zes_led_state_t ledState{};
      zes_led_color_t ledColor{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumLeds(deviceHandle, &count, &ledHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumLeds", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesLedGetProperties(ledHandle, &ledProperties));
      EXPECT_TRUE(compare_env("zesLedGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesLedGetState(ledHandle, &ledState));
      EXPECT_TRUE(compare_env("zesLedGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesLedSetState(ledHandle, ze_bool_t(1)));
      EXPECT_TRUE(compare_env("zesLedSetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesLedSetColor(ledHandle, &ledColor));
      EXPECT_TRUE(compare_env("zesLedSetColor", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingRasManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_ras_handle_t rasHandle{};
      zes_ras_properties_t rasProperties{};
      zes_ras_config_t rasConfig{};
      zes_ras_state_t rasState{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumRasErrorSets(deviceHandle, &count, &rasHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumRasErrorSets", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasGetProperties(rasHandle, &rasProperties));
      EXPECT_TRUE(compare_env("zesRasGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasGetConfig(rasHandle, &rasConfig));
      EXPECT_TRUE(compare_env("zesRasGetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasSetConfig(rasHandle, &rasConfig));
      EXPECT_TRUE(compare_env("zesRasSetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasGetState(rasHandle, ze_bool_t(0), &rasState));
      EXPECT_TRUE(compare_env("zesRasGetState", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingDiagnosticsManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_diag_handle_t diagHandle{};
      zes_diag_properties_t diagProperties{};
      zes_diag_test_t diagTest{};
      zes_diag_result_t diagResult{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumDiagnosticTestSuites(deviceHandle, &count, &diagHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumDiagnosticTestSuites", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDiagnosticsGetProperties(diagHandle, &diagProperties));
      EXPECT_TRUE(compare_env("zesDiagnosticsGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDiagnosticsGetTests(diagHandle, &count, &diagTest));
      EXPECT_TRUE(compare_env("zesDiagnosticsGetTests", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDiagnosticsRunTests(diagHandle, 0, 1, &diagResult));
      EXPECT_TRUE(compare_env("zesDiagnosticsRunTests", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingSchedulerManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_sched_handle_t schedHandle{};
      zes_sched_properties_t schedProperties{};
      zes_sched_mode_t schedMode{};
      zes_sched_timeout_properties_t timeoutProps{};
      zes_sched_timeslice_properties_t timesliceProps{};
      ze_bool_t needReload = ze_bool_t(0);

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumSchedulers(deviceHandle, &count, &schedHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumSchedulers", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerGetProperties(schedHandle, &schedProperties));
      EXPECT_TRUE(compare_env("zesSchedulerGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerGetCurrentMode(schedHandle, &schedMode));
      EXPECT_TRUE(compare_env("zesSchedulerGetCurrentMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerGetTimeoutModeProperties(schedHandle, ze_bool_t(0), &timeoutProps));
      EXPECT_TRUE(compare_env("zesSchedulerGetTimeoutModeProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerGetTimesliceModeProperties(schedHandle, ze_bool_t(0), &timesliceProps));
      EXPECT_TRUE(compare_env("zesSchedulerGetTimesliceModeProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerSetTimeoutMode(schedHandle, &timeoutProps, &needReload));
      EXPECT_TRUE(compare_env("zesSchedulerSetTimeoutMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerSetTimesliceMode(schedHandle, &timesliceProps, &needReload));
      EXPECT_TRUE(compare_env("zesSchedulerSetTimesliceMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerSetExclusiveMode(schedHandle, &needReload));
      EXPECT_TRUE(compare_env("zesSchedulerSetExclusiveMode", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingEccManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      ze_bool_t available = ze_bool_t(0);
      ze_bool_t configurable = ze_bool_t(0);
      zes_device_ecc_desc_t eccDesc{};
      zes_device_ecc_properties_t eccState{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEccAvailable(deviceHandle, &available));
      EXPECT_TRUE(compare_env("zesDeviceEccAvailable", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEccConfigurable(deviceHandle, &configurable));
      EXPECT_TRUE(compare_env("zesDeviceEccConfigurable", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGetEccState(deviceHandle, &eccState));
      EXPECT_TRUE(compare_env("zesDeviceGetEccState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceSetEccState(deviceHandle, &eccDesc, &eccState));
      EXPECT_TRUE(compare_env("zesDeviceSetEccState", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingPerformanceFactorManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_perf_handle_t perfHandle{};
      zes_perf_properties_t perfProperties{};
      double perfFactor = 0.0;

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumPerformanceFactorDomains(deviceHandle, &count, &perfHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumPerformanceFactorDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPerformanceFactorGetProperties(perfHandle, &perfProperties));
      EXPECT_TRUE(compare_env("zesPerformanceFactorGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPerformanceFactorGetConfig(perfHandle, &perfFactor));
      EXPECT_TRUE(compare_env("zesPerformanceFactorGetConfig", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPerformanceFactorSetConfig(perfHandle, perfFactor));
      EXPECT_TRUE(compare_env("zesPerformanceFactorSetConfig", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFirmwareManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_firmware_handle_t firmwareHandle{};
      zes_firmware_properties_t firmwareProperties{};
      uint32_t completion = 0;
      size_t logSize = 0;

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumFirmwares(deviceHandle, &count, &firmwareHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumFirmwares", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFirmwareGetProperties(firmwareHandle, &firmwareProperties));
      EXPECT_TRUE(compare_env("zesFirmwareGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFirmwareFlash(firmwareHandle, nullptr, 0));
      EXPECT_TRUE(compare_env("zesFirmwareFlash", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFirmwareGetFlashProgress(firmwareHandle, &completion));
      EXPECT_TRUE(compare_env("zesFirmwareGetFlashProgress", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFirmwareGetConsoleLogs(firmwareHandle, &logSize, nullptr));
      EXPECT_TRUE(compare_env("zesFirmwareGetConsoleLogs", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingPciManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_pci_properties_t pciProperties{};
      zes_pci_state_t pciState{};
      zes_pci_bar_properties_t pciBarProps{};
      zes_pci_stats_t pciStats{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDevicePciGetProperties(deviceHandle, &pciProperties));
      EXPECT_TRUE(compare_env("zesDevicePciGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDevicePciGetState(deviceHandle, &pciState));
      EXPECT_TRUE(compare_env("zesDevicePciGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDevicePciGetBars(deviceHandle, &count, &pciBarProps));
      EXPECT_TRUE(compare_env("zesDevicePciGetBars", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDevicePciGetStats(deviceHandle, &pciStats));
      EXPECT_TRUE(compare_env("zesDevicePciGetStats", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingEventManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      zes_event_type_flags_t events = 0;
      uint32_t numDeviceEvents = 0;
      zes_event_type_flags_t deviceEvents = 0;
      uint32_t timeout = 1000;

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEventRegister(deviceHandle, events));
      EXPECT_TRUE(compare_env("zesDeviceEventRegister", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverEventListen(driverHandles[i], timeout, 1, &deviceHandle, &numDeviceEvents, &deviceEvents));
      EXPECT_TRUE(compare_env("zesDriverEventListen", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverEventListenEx(driverHandles[i], timeout, 1, &deviceHandle, &numDeviceEvents, &deviceEvents));
      EXPECT_TRUE(compare_env("zesDriverEventListenEx", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingOverclockingManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      zes_overclock_mode_t overclockMode = ZES_OVERCLOCK_MODE_MODE_OFF;
      ze_bool_t waiver = ze_bool_t(0);
      ze_bool_t overclockState = ze_bool_t(0);
      zes_pending_action_t pendingAction{};
      ze_bool_t pendingReset = ze_bool_t(0);
      uint32_t overclockDomains = 0;
      uint32_t availableControls = 0;
      ze_bool_t restart = ze_bool_t(0);

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceSetOverclockWaiver(deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceSetOverclockWaiver", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGetOverclockDomains(deviceHandle, &overclockDomains));
      EXPECT_TRUE(compare_env("zesDeviceGetOverclockDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGetOverclockControls(deviceHandle, ZES_OVERCLOCK_DOMAIN_CARD, &availableControls));
      EXPECT_TRUE(compare_env("zesDeviceGetOverclockControls", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceResetOverclockSettings(deviceHandle, restart));
      EXPECT_TRUE(compare_env("zesDeviceResetOverclockSettings", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceReadOverclockState(deviceHandle, &overclockMode, &waiver, &overclockState, &pendingAction, &pendingReset));
      EXPECT_TRUE(compare_env("zesDeviceReadOverclockState", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingDeviceManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      zes_device_properties_t deviceProperties{};
      zes_device_state_t deviceState{};
      uint32_t processCount = 0;

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGetProperties(deviceHandle, &deviceProperties));
      EXPECT_TRUE(compare_env("zesDeviceGetProperties", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGetState(deviceHandle, &deviceState));
      EXPECT_TRUE(compare_env("zesDeviceGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceProcessesGetState(deviceHandle, &processCount, nullptr));
      EXPECT_TRUE(compare_env("zesDeviceProcessesGetState", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceReset(deviceHandle, ze_bool_t(0)));
      EXPECT_TRUE(compare_env("zesDeviceReset", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceResetExt(deviceHandle, nullptr));
      EXPECT_TRUE(compare_env("zesDeviceResetExt", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingExtendedPowerManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_pwr_handle_t powerHandle{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumPowerDomains(deviceHandle, &count, &powerHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumPowerDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGetCardPowerDomain(deviceHandle, &powerHandle));
      EXPECT_TRUE(compare_env("zesDeviceGetCardPowerDomain", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerGetLimitsExt(powerHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesPowerGetLimitsExt", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesPowerSetLimitsExt(powerHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesPowerSetLimitsExt", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingExtendedEngineManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_engine_handle_t engineHandle{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumEngineGroups(deviceHandle, &count, &engineHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumEngineGroups", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesEngineGetActivityExt(engineHandle, &count, nullptr));
      EXPECT_TRUE(compare_env("zesEngineGetActivityExt", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFrequencyOverclockingApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_freq_handle_t freqHandle{};
      zes_oc_capabilities_t ocCapabilities{};
      double frequencyTarget = 1000.0;
      double voltageTarget = 1.0;
      double voltageOffset = 0.1;
      double iccMax = 100.0;
      double tjMax = 85.0;
      zes_oc_mode_t ocMode{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumFrequencyDomains(deviceHandle, &count, &freqHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumFrequencyDomains", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcGetCapabilities(freqHandle, &ocCapabilities));
      EXPECT_TRUE(compare_env("zesFrequencyOcGetCapabilities", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcGetFrequencyTarget(freqHandle, &frequencyTarget));
      EXPECT_TRUE(compare_env("zesFrequencyOcGetFrequencyTarget", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcSetFrequencyTarget(freqHandle, frequencyTarget));
      EXPECT_TRUE(compare_env("zesFrequencyOcSetFrequencyTarget", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcGetVoltageTarget(freqHandle, &voltageTarget, &voltageOffset));
      EXPECT_TRUE(compare_env("zesFrequencyOcGetVoltageTarget", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcSetVoltageTarget(freqHandle, voltageTarget, voltageOffset));
      EXPECT_TRUE(compare_env("zesFrequencyOcSetVoltageTarget", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcGetMode(freqHandle, &ocMode));
      EXPECT_TRUE(compare_env("zesFrequencyOcGetMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcSetMode(freqHandle, ZES_OC_MODE_OFF));
      EXPECT_TRUE(compare_env("zesFrequencyOcSetMode", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcGetIccMax(freqHandle, &iccMax));
      EXPECT_TRUE(compare_env("zesFrequencyOcGetIccMax", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcSetIccMax(freqHandle, iccMax));
      EXPECT_TRUE(compare_env("zesFrequencyOcSetIccMax", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcGetTjMax(freqHandle, &tjMax));
      EXPECT_TRUE(compare_env("zesFrequencyOcGetTjMax", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFrequencyOcSetTjMax(freqHandle, tjMax));
      EXPECT_TRUE(compare_env("zesFrequencyOcSetTjMax", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingExtendedRasManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_ras_handle_t rasHandle{};
      uint32_t categoryCount = 0;
      zes_ras_state_exp_t rasStateExp{};

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumRasErrorSets(deviceHandle, &count, &rasHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumRasErrorSets", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasGetStateExp(rasHandle, &categoryCount, nullptr));
      EXPECT_TRUE(compare_env("zesRasGetStateExp", std::to_string(i + 1)));
      if (categoryCount > 0)
      {
        categoryCount = 1; // Limit to 1 for testing
        EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasGetStateExp(rasHandle, &categoryCount, &rasStateExp));
        EXPECT_TRUE(compare_env("zesRasGetStateExp", std::to_string(i + 1)));
      }
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesRasClearStateExp(rasHandle, ZES_RAS_ERROR_CATEGORY_EXP_RESET));
      EXPECT_TRUE(compare_env("zesRasClearStateExp", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingExtendedFirmwareManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_firmware_handle_t firmwareHandle{};
      char securityVersion[ZES_STRING_PROPERTY_SIZE];

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumFirmwares(deviceHandle, &count, &firmwareHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumFirmwares", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFirmwareGetSecurityVersionExp(firmwareHandle, securityVersion));
      EXPECT_TRUE(compare_env("zesFirmwareGetSecurityVersionExp", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesFirmwareSetSecurityVersionExp(firmwareHandle));
      EXPECT_TRUE(compare_env("zesFirmwareSetSecurityVersionExp", std::to_string(i + 1)));
    }
  }

  TEST(
      SysManApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingSchedulerDebugManagementApisThenExpectNullDriverIsReachedSuccessfully)
  {
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    uint32_t driverCount = 0;
    std::vector<zes_driver_handle_t> driverHandles{};
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, nullptr));
    EXPECT_GT(driverCount, 0);
    driverHandles.resize(driverCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&driverCount, driverHandles.data()));

    for (std::size_t i = 0; i < driverHandles.size(); i++)
    {
      uint32_t deviceCount = 1;
      zes_device_handle_t deviceHandle{};
      uint32_t count = 1;
      zes_sched_handle_t schedHandle{};
      ze_bool_t needReload = ze_bool_t(0);

      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceGet(driverHandles[i], &deviceCount, &deviceHandle));
      EXPECT_TRUE(compare_env("zesDeviceGet", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesDeviceEnumSchedulers(deviceHandle, &count, &schedHandle));
      EXPECT_TRUE(compare_env("zesDeviceEnumSchedulers", std::to_string(i + 1)));
      EXPECT_EQ(ZE_RESULT_SUCCESS, zesSchedulerSetComputeUnitDebugMode(schedHandle, &needReload));
      EXPECT_TRUE(compare_env("zesSchedulerSetComputeUnitDebugMode", std::to_string(i + 1)));
    }
  }
  
} // namespace
