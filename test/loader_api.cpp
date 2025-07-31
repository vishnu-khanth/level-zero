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

namespace
{

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
      GivenLevelZeroLoaderPresentWhenCallingzeGetLoaderVersionsAPIThenValidVersionIsReturned)
  {

    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));

    size_t size = 0;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderGetVersions(&size, nullptr));
    EXPECT_GT(size, 0);

    std::vector<zel_component_version_t> versions(size);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderGetVersions(&size, versions.data()));

    std::cout << "Found " << versions.size() << " versions" << std::endl;
    std::cout << std::endl;
    const std::string loader_name = "loader";
    for (auto &component : versions)
    {
      std::cout << "component.component_name: " << component.component_name << std::endl;
      std::cout << "component.component_lib_version.major: " << component.component_lib_version.major << std::endl;
      std::cout << "component.spec_version: " << component.spec_version << std::endl;
      std::cout << "component.component_lib_name: " << component.component_name << std::endl;
      std::cout << std::endl;

      if (loader_name == component.component_name)
      {
        EXPECT_GE(component.component_lib_version.major, 1);
      }
    }
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithTypesUnsupportedWithFailureThenSupportedTypesThenSuccessReturned)
  {

    uint32_t pCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_NPU;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=GPU"));
    EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pCount, nullptr, &desc));
    EXPECT_EQ(pCount, 0);
    pCount = 0;
    desc.flags = UINT32_MAX;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pCount, nullptr, &desc));
    EXPECT_GT(pCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithGPUTypeThenExpectPassWithGPUorAllOnly)
  {

    uint32_t pCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_GPU;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=GPU"));
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
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithNPUTypeThenExpectPassWithNPUorAllOnly)
  {

    uint32_t pCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_NPU;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=NPU"));
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
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversWithAnyTypeWithNullDriverAcceptingAllThenExpectatLeast1Driver)
  {

    uint32_t pCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = ZE_INIT_DRIVER_TYPE_FLAG_NPU;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=ALL"));
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
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversThenzeInitThenBothCallsSucceedWithAllTypes)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=ALL"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversThenzeInitThenBothCallsSucceedWithGPUTypes)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=GPU"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_GPU_ONLY));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenZeInitDriversUnsupportedOnTheDriverWhenCallingZeInitDriversThenUninitializedReturned)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_MISSING_API=zeInitDrivers"));
    EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_EQ(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingZeInitDriversThenzeInitThenBothCallsSucceedWithNPUTypes)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=NPU"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_VPU_ONLY));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingzeInitThenZeInitDriversThenBothCallsSucceedWithAllTypes)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=ALL"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingzeInitThenZeInitDriversThenBothCallsSucceedWithGPUTypes)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=GPU"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_GPU_ONLY));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWhenCallingzeInitThenZeInitDriversThenBothCallsSucceedWithNPUTypes)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_TYPE=NPU"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(ZE_INIT_FLAG_VPU_ONLY));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWithMultipleDriversMissingInitDriversWhenCallingZeInitDriversThenExpectSuccessForZeInit)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_MISSING_API=zeInitDrivers"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
    EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_EQ(pInitDriversCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWithMultipleDriversMissingInitDriversInOneDriverWhenCallingZeInitDriversThenExpectSuccessForZeInitDrivers)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_MISSING_API_DRIVER_ID=zeInitDrivers:1"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_GT(pInitDriversCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWithMultipleDriversMissingInitDriversWhenCallingZeInitDriversThenExpectSuccessForZeInitWithDriverGetAfterInitDrivers)
  {

    uint32_t pInitDriversCount = 0;
    uint32_t pDriverGetCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZEL_TEST_MISSING_API=zeInitDrivers"));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    EXPECT_EQ(pInitDriversCount, 0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZeInitDriversThenExpectSuccessForZeInit)
  {

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
      GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZeInitThenZeInitDriversThenExpectSuccessForZeInitWithDriverGetAfterInitDrivers)
  {

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
      GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZesInitThenExpectSuccessForZesDriverGet)
  {

    uint32_t pDriverGetCount = 0;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesInit(0));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zesDriverGet(&pDriverGetCount, nullptr));
    EXPECT_GT(pDriverGetCount, 0);
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingZesInitThenZeInitDriversExpectSuccessForZesDriverGetAndZeInitDrivers)
  {

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
      GivenLoaderNotInDestructionStateWhenCallingzelCheckIsLoaderInTearDownThenFalseIsReturned)
  {

    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInit(0));
    EXPECT_FALSE(zelCheckIsLoaderInTearDown());
    EXPECT_FALSE(zelCheckIsLoaderInTearDown());
    EXPECT_FALSE(zelCheckIsLoaderInTearDown());
    EXPECT_FALSE(zelCheckIsLoaderInTearDown());
  }

  class CaptureOutput
  {
  private:
    int original_fd;
    int fd;
    int stream;
    char filename[50] = "/tmp/capture_output_XXXXXX";

  public:
    enum
    {
      Stdout = 1,
      Stderr = 2
    };

    CaptureOutput(int stream_) : stream(stream_)
    {
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

    ~CaptureOutput()
    {
      if (original_fd != -1)
      {
        fflush(nullptr);
        _dup2(original_fd, stream);
        _close(original_fd);
        original_fd = -1;
      }
      if (remove(filename) != 0)
      {
        std::cerr << "Deleting file " << filename << " failed.";
      }
    }

    std::string GetOutput()
    {
      if (original_fd != -1)
      {
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
      GivenZeInitDriverWhenCalledThenNoOutputIsPrintedToStdout)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;

    CaptureOutput capture(CaptureOutput::Stdout);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));

    std::string output = capture.GetOutput();
    EXPECT_TRUE(output.empty());
  }

  TEST(
      LoaderInit,
      GivenLevelZeroLoaderPresentWithMultipleDriversWhenCallingDriverGetPropertiesThenExpectSuccess)
  {

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
    for (uint32_t i = 0; i < pDriverGetCount; ++i)
    {
      ze_driver_properties_t driverProperties = {ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[i], &driverProperties));
      std::cout << "Driver " << i << " properties:" << std::endl;
      std::cout << "  Driver version: " << driverProperties.driverVersion << std::endl;
      std::cout << "  UUID: ";
      for (auto byte : driverProperties.uuid.id)
      {
        std::cout << std::hex << static_cast<int>(byte);
      }
      std::cout << std::dec << std::endl;
    }
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForModule)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE, module, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, module);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForModule)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE, module, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, module);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForModuleBuildLog)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE_BUILD_LOG, buildLog, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, buildLog);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleBuildLogDestroy(buildLog));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForModuleBuildLog)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_MODULE_BUILD_LOG, buildLog, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, buildLog);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleBuildLogDestroy(buildLog));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForKernel)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_KERNEL, kernel, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, kernel);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForKernel)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_KERNEL, kernel, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, kernel);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForSampler)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_SAMPLER, sampler, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, sampler);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerDestroy(sampler));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForSampler)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_SAMPLER, sampler, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, sampler);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerDestroy(sampler));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForPhysicalMem)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_PHYSICAL_MEM, physicalMem, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, physicalMem);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemDestroy(context, physicalMem));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForPhysicalMem)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_PHYSICAL_MEM, physicalMem, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, physicalMem);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemDestroy(context, physicalMem));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForFence)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_FENCE, fence, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, fence);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceDestroy(fence));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForFence)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_FENCE, fence, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, fence);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceDestroy(fence));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForEventPool)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT_POOL, eventPool, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, eventPool);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForEventPool)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT_POOL, eventPool, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, eventPool);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForImage)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_IMAGE, image, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, image);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageDestroy(image));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForImage)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_IMAGE, image, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, image);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageDestroy(image));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForContext)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_CONTEXT, context, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, context);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForContext)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_CONTEXT, context, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, context);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForCommandQueue)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_QUEUE, commandQueue, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, commandQueue);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForCommandQueue)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_QUEUE, commandQueue, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, commandQueue);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(commandQueue));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForCommandList)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_LIST, commandList, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, commandList);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListDestroy(commandList));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForCommandList)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_COMMAND_LIST, commandList, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, commandList);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListDestroy(commandList));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForEvent)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT, event, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, event);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForEvent)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_EVENT, event, reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, event);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForDriver)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DRIVER, drivers[0], reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, drivers[0]);
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForDriver)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DRIVER, drivers[0], reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_EQ(translatedHandle, drivers[0]);
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportDisabledThenExpectHandleTranslationForDevice)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=1"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DEVICE, devices[0], reinterpret_cast<void **>(&translatedHandle)));
    EXPECT_NE(translatedHandle, devices[0]);
  }

  TEST(
      LoaderTranslateHandles,
      GivenLevelZeroLoaderPresentWhenCallingZelLoaderTranslateHandleInternalWithInterceptEnabledAndDDiSupportEnabledThenExpectNoHandleTranslationForDevice)
  {

    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    putenv_safe(const_cast<char *>("ZE_ENABLE_LOADER_INTERCEPT=1"));
    putenv_safe(const_cast<char *>("ZEL_TEST_NULL_DRIVER_DISABLE_DDI_EXT=0"));
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
    EXPECT_EQ(ZE_RESULT_SUCCESS, zelLoaderTranslateHandle(ZEL_HANDLE_DEVICE, devices[0], reinterpret_cast<void **>(&translatedHandle)));
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

  // Core API Module Tests
  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingDriverApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      // zeDriver APIs
      ze_api_version_t version;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetApiVersion(drivers[i], &version));
      EXPECT_TRUE(compare_env("zeDriverGetApiVersion", std::to_string(i + 1)));

      ze_driver_properties_t driverProps = {ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetProperties(drivers[i], &driverProps));
      EXPECT_TRUE(compare_env("zeDriverGetProperties", std::to_string(i + 1)));

      ze_driver_ipc_properties_t ipcProps = {ZE_STRUCTURE_TYPE_DRIVER_IPC_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetIpcProperties(drivers[i], &ipcProps));
      EXPECT_TRUE(compare_env("zeDriverGetIpcProperties", std::to_string(i + 1)));

      uint32_t extCount = 0;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetExtensionProperties(drivers[i], &extCount, nullptr));
      EXPECT_TRUE(compare_env("zeDriverGetExtensionProperties", std::to_string(i + 1)));

      void *funcPtr = nullptr;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetExtensionFunctionAddress(drivers[i], "test", &funcPtr));
      EXPECT_TRUE(compare_env("zeDriverGetExtensionFunctionAddress", std::to_string(i + 1)));

      const char *errorDesc = nullptr;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverGetLastErrorDescription(drivers[i], &errorDesc));
      EXPECT_TRUE(compare_env("zeDriverGetLastErrorDescription", std::to_string(i + 1)));
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingDeviceApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));
      EXPECT_TRUE(compare_env("zeDeviceGet", std::to_string(i + 1)));

      // zeDevice APIs
      ze_device_properties_t deviceProps = {ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetProperties(devices[0], &deviceProps));
      EXPECT_TRUE(compare_env("zeDeviceGetProperties", std::to_string(i + 1)));

      ze_device_compute_properties_t computeProps = {ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetComputeProperties(devices[0], &computeProps));
      EXPECT_TRUE(compare_env("zeDeviceGetComputeProperties", std::to_string(i + 1)));

      uint32_t memCount = 0;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetMemoryProperties(devices[0], &memCount, nullptr));
      EXPECT_TRUE(compare_env("zeDeviceGetMemoryProperties", std::to_string(i + 1)));

      ze_device_memory_access_properties_t memAccessProps = {ZE_STRUCTURE_TYPE_DEVICE_MEMORY_ACCESS_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetMemoryAccessProperties(devices[0], &memAccessProps));
      EXPECT_TRUE(compare_env("zeDeviceGetMemoryAccessProperties", std::to_string(i + 1)));

      uint32_t cacheCount = 0;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetCacheProperties(devices[0], &cacheCount, nullptr));
      EXPECT_TRUE(compare_env("zeDeviceGetCacheProperties", std::to_string(i + 1)));

      ze_device_image_properties_t imageProps = {ZE_STRUCTURE_TYPE_DEVICE_IMAGE_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetImageProperties(devices[0], &imageProps));
      EXPECT_TRUE(compare_env("zeDeviceGetImageProperties", std::to_string(i + 1)));

      uint32_t queueCount = 0;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetCommandQueueGroupProperties(devices[0], &queueCount, nullptr));
      EXPECT_TRUE(compare_env("zeDeviceGetCommandQueueGroupProperties", std::to_string(i + 1)));

      ze_device_external_memory_properties_t extMemProps = {ZE_STRUCTURE_TYPE_DEVICE_EXTERNAL_MEMORY_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetExternalMemoryProperties(devices[0], &extMemProps));
      EXPECT_TRUE(compare_env("zeDeviceGetExternalMemoryProperties", std::to_string(i + 1)));

      ze_device_p2p_properties_t p2pProps = {ZE_STRUCTURE_TYPE_DEVICE_P2P_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetP2PProperties(devices[0], devices[0], &p2pProps));
      EXPECT_TRUE(compare_env("zeDeviceGetP2PProperties", std::to_string(i + 1)));

      ze_bool_t canAccess = false;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceCanAccessPeer(devices[0], devices[0], &canAccess));
      EXPECT_TRUE(compare_env("zeDeviceCanAccessPeer", std::to_string(i + 1)));

      uint64_t hostTimestamp, deviceTimestamp;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetGlobalTimestamps(devices[0], &hostTimestamp, &deviceTimestamp));
      EXPECT_TRUE(compare_env("zeDeviceGetGlobalTimestamps", std::to_string(i + 1)));
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingContextApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      // zeContext APIs
      ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
      ze_context_handle_t context;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));
      EXPECT_TRUE(compare_env("zeContextCreate", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextSystemBarrier(context, nullptr));
      EXPECT_TRUE(compare_env("zeContextSystemBarrier", std::to_string(i + 1)));

      // Memory residency APIs
      void *ptr = nullptr;
      size_t size = 1024;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextMakeMemoryResident(context, nullptr, ptr, size));
      EXPECT_TRUE(compare_env("zeContextMakeMemoryResident", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextEvictMemory(context, nullptr, ptr, size));
      EXPECT_TRUE(compare_env("zeContextEvictMemory", std::to_string(i + 1)));

      // Image residency APIs
      ze_image_handle_t image = nullptr;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextMakeImageResident(context, nullptr, image));
      EXPECT_TRUE(compare_env("zeContextMakeImageResident", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextEvictImage(context, nullptr, image));
      EXPECT_TRUE(compare_env("zeContextEvictImage", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      EXPECT_TRUE(compare_env("zeContextDestroy", std::to_string(i + 1)));
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingCommandQueueApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeCommandQueue APIs
        ze_command_queue_desc_t queueDesc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC};
        ze_command_queue_handle_t queue;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, devices[0], &queueDesc, &queue));
        EXPECT_TRUE(compare_env("zeCommandQueueCreate", std::to_string(i + 1)));

        uint32_t ordinal;
        uint32_t index;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueGetOrdinal(queue, &ordinal));
        EXPECT_TRUE(compare_env("zeCommandQueueGetOrdinal", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueGetIndex(queue, &index));
        EXPECT_TRUE(compare_env("zeCommandQueueGetIndex", std::to_string(i + 1)));

        std::vector<ze_command_list_handle_t> cmdLists;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueExecuteCommandLists(queue, 0, nullptr, nullptr));
        EXPECT_TRUE(compare_env("zeCommandQueueExecuteCommandLists", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueSynchronize(queue, UINT64_MAX));
        EXPECT_TRUE(compare_env("zeCommandQueueSynchronize", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(queue));
        EXPECT_TRUE(compare_env("zeCommandQueueDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingCommandListApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeCommandList APIs
        ze_command_list_desc_t cmdListDesc = {ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC};
        ze_command_list_handle_t cmdList;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListCreate(context, devices[0], &cmdListDesc, &cmdList));
        EXPECT_TRUE(compare_env("zeCommandListCreate", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListClose(cmdList));
        EXPECT_TRUE(compare_env("zeCommandListClose", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListReset(cmdList));
        EXPECT_TRUE(compare_env("zeCommandListReset", std::to_string(i + 1)));

        // Basic append operations
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendBarrier(cmdList, nullptr, 0, nullptr));
        EXPECT_TRUE(compare_env("zeCommandListAppendBarrier", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendMemoryRangesBarrier(cmdList, 0, nullptr, nullptr, 0, 0, nullptr));
        EXPECT_TRUE(compare_env("zeCommandListAppendMemoryRangesBarrier", std::to_string(i + 1)));

        void *srcPtr = nullptr;
        void *dstPtr = nullptr;
        size_t size = 1024;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendMemoryCopy(cmdList, dstPtr, srcPtr, size, nullptr, 0, nullptr));
        EXPECT_TRUE(compare_env("zeCommandListAppendMemoryCopy", std::to_string(i + 1)));

        uint32_t pattern = 0x12345678;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendMemoryFill(cmdList, dstPtr, &pattern, sizeof(pattern), size, nullptr, 0, nullptr));
        EXPECT_TRUE(compare_env("zeCommandListAppendMemoryFill", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListDestroy(cmdList));
        EXPECT_TRUE(compare_env("zeCommandListDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingMemoryApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeMem APIs
        ze_device_mem_alloc_desc_t deviceDesc = {ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC};
        void *devicePtr = nullptr;
        size_t size = 1024;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemAllocDevice(context, &deviceDesc, size, 0, devices[0], &devicePtr));
        EXPECT_TRUE(compare_env("zeMemAllocDevice", std::to_string(i + 1)));

        ze_host_mem_alloc_desc_t hostDesc = {ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC};
        void *hostPtr = nullptr;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemAllocHost(context, &hostDesc, size, 0, &hostPtr));
        EXPECT_TRUE(compare_env("zeMemAllocHost", std::to_string(i + 1)));

        void *sharedPtr = nullptr;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemAllocShared(context, &deviceDesc, &hostDesc, size, 0, devices[0], &sharedPtr));
        EXPECT_TRUE(compare_env("zeMemAllocShared", std::to_string(i + 1)));

        // Memory properties
        ze_memory_allocation_properties_t memProps = {ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES};
        ze_device_handle_t allocDevice;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemGetAllocProperties(context, devicePtr, &memProps, &allocDevice));
        EXPECT_TRUE(compare_env("zeMemGetAllocProperties", std::to_string(i + 1)));

        void *basePtr;
        size_t allocSize;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemGetAddressRange(context, devicePtr, &basePtr, &allocSize));
        EXPECT_TRUE(compare_env("zeMemGetAddressRange", std::to_string(i + 1)));

        // IPC operations
        ze_ipc_mem_handle_t ipcHandle;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemGetIpcHandle(context, devicePtr, &ipcHandle));
        EXPECT_TRUE(compare_env("zeMemGetIpcHandle", std::to_string(i + 1)));

        void *ipcPtr;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemOpenIpcHandle(context, devices[0], ipcHandle, 0, &ipcPtr));
        EXPECT_TRUE(compare_env("zeMemOpenIpcHandle", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemCloseIpcHandle(context, ipcPtr));
        EXPECT_TRUE(compare_env("zeMemCloseIpcHandle", std::to_string(i + 1)));

        // Memory management
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemFree(context, devicePtr));
        EXPECT_TRUE(compare_env("zeMemFree", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemFree(context, hostPtr));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemFree(context, sharedPtr));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingEventApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeEventPool APIs
        ze_event_pool_desc_t poolDesc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC};
        poolDesc.count = 1;
        ze_event_pool_handle_t eventPool;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &poolDesc, 1, &devices[0], &eventPool));
        EXPECT_TRUE(compare_env("zeEventPoolCreate", std::to_string(i + 1)));

        ze_ipc_event_pool_handle_t poolIpcHandle;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolGetIpcHandle(eventPool, &poolIpcHandle));
        EXPECT_TRUE(compare_env("zeEventPoolGetIpcHandle", std::to_string(i + 1)));

        ze_event_pool_handle_t ipcPool;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolOpenIpcHandle(context, poolIpcHandle, &ipcPool));
        EXPECT_TRUE(compare_env("zeEventPoolOpenIpcHandle", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCloseIpcHandle(ipcPool));
        EXPECT_TRUE(compare_env("zeEventPoolCloseIpcHandle", std::to_string(i + 1)));

        // zeEvent APIs
        ze_event_desc_t eventDesc = {ZE_STRUCTURE_TYPE_EVENT_DESC};
        eventDesc.index = 0;
        ze_event_handle_t event;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventCreate(eventPool, &eventDesc, &event));
        EXPECT_TRUE(compare_env("zeEventCreate", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventHostSignal(event));
        EXPECT_TRUE(compare_env("zeEventHostSignal", std::to_string(i + 1)));

        zeEventQueryStatus(event);
        EXPECT_TRUE(compare_env("zeEventQueryStatus", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventHostSynchronize(event, UINT64_MAX));
        EXPECT_TRUE(compare_env("zeEventHostSynchronize", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventHostReset(event));
        EXPECT_TRUE(compare_env("zeEventHostReset", std::to_string(i + 1)));

        ze_kernel_timestamp_result_t timestamp;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventQueryKernelTimestamp(event, &timestamp));
        EXPECT_TRUE(compare_env("zeEventQueryKernelTimestamp", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
        EXPECT_TRUE(compare_env("zeEventDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
        EXPECT_TRUE(compare_env("zeEventPoolDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingModuleApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeModule APIs
        ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
        ze_module_handle_t module;
        ze_module_build_log_handle_t buildLog;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, &buildLog));
        EXPECT_TRUE(compare_env("zeModuleCreate", std::to_string(i + 1)));

        ze_module_properties_t moduleProps = {ZE_STRUCTURE_TYPE_MODULE_PROPERTIES};
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleGetProperties(module, &moduleProps));
        EXPECT_TRUE(compare_env("zeModuleGetProperties", std::to_string(i + 1)));

        uint32_t count = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleGetKernelNames(module, &count, nullptr));
        EXPECT_TRUE(compare_env("zeModuleGetKernelNames", std::to_string(i + 1)));

        size_t nativeSize = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleGetNativeBinary(module, &nativeSize, nullptr));
        EXPECT_TRUE(compare_env("zeModuleGetNativeBinary", std::to_string(i + 1)));

        void *functionPtr;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleGetFunctionPointer(module, "test", &functionPtr));
        EXPECT_TRUE(compare_env("zeModuleGetFunctionPointer", std::to_string(i + 1)));

        void *globalPtr;
        size_t globalSize;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleGetGlobalPointer(module, "global", &globalSize, &globalPtr));
        EXPECT_TRUE(compare_env("zeModuleGetGlobalPointer", std::to_string(i + 1)));

        // zeModuleBuildLog APIs
        if (buildLog)
        {
          size_t logSize = 0;
          EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleBuildLogGetString(buildLog, &logSize, nullptr));
          EXPECT_TRUE(compare_env("zeModuleBuildLogGetString", std::to_string(i + 1)));

          EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleBuildLogDestroy(buildLog));
          EXPECT_TRUE(compare_env("zeModuleBuildLogDestroy", std::to_string(i + 1)));
        }

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
        EXPECT_TRUE(compare_env("zeModuleDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingKernelApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
        ze_module_handle_t module;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, nullptr));

        // zeKernel APIs
        ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC};
        kernelDesc.pKernelName = "test";
        ze_kernel_handle_t kernel;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelCreate(module, &kernelDesc, &kernel));
        EXPECT_TRUE(compare_env("zeKernelCreate", std::to_string(i + 1)));

        ze_kernel_properties_t kernelProps = {ZE_STRUCTURE_TYPE_KERNEL_PROPERTIES};
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelGetProperties(kernel, &kernelProps));
        EXPECT_TRUE(compare_env("zeKernelGetProperties", std::to_string(i + 1)));

        size_t kernelNameSize = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelGetName(kernel, &kernelNameSize, nullptr));
        EXPECT_TRUE(compare_env("zeKernelGetName", std::to_string(i + 1)));

        uint32_t groupSizeX = 1, groupSizeY = 1, groupSizeZ = 1;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetGroupSize(kernel, groupSizeX, groupSizeY, groupSizeZ));
        EXPECT_TRUE(compare_env("zeKernelSetGroupSize", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSuggestGroupSize(kernel, 1024, 1, 1, &groupSizeX, &groupSizeY, &groupSizeZ));
        EXPECT_TRUE(compare_env("zeKernelSuggestGroupSize", std::to_string(i + 1)));

        uint32_t maxGroupSize;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSuggestMaxCooperativeGroupCount(kernel, &maxGroupSize));
        EXPECT_TRUE(compare_env("zeKernelSuggestMaxCooperativeGroupCount", std::to_string(i + 1)));

        void *argValue = nullptr;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetArgumentValue(kernel, 0, sizeof(void *), &argValue));
        EXPECT_TRUE(compare_env("zeKernelSetArgumentValue", std::to_string(i + 1)));

        ze_kernel_indirect_access_flags_t flags;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelGetIndirectAccess(kernel, &flags));
        EXPECT_TRUE(compare_env("zeKernelGetIndirectAccess", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetIndirectAccess(kernel, ZE_KERNEL_INDIRECT_ACCESS_FLAG_HOST));
        EXPECT_TRUE(compare_env("zeKernelSetIndirectAccess", std::to_string(i + 1)));

        uint32_t attrCount = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelGetSourceAttributes(kernel, &attrCount, nullptr));
        EXPECT_TRUE(compare_env("zeKernelGetSourceAttributes", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetCacheConfig(kernel, ZE_CACHE_CONFIG_FLAG_LARGE_SLM));
        EXPECT_TRUE(compare_env("zeKernelSetCacheConfig", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel));
        EXPECT_TRUE(compare_env("zeKernelDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingImageApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeImage APIs
        ze_image_desc_t imageDesc = {ZE_STRUCTURE_TYPE_IMAGE_DESC};
        imageDesc.type = ZE_IMAGE_TYPE_2D;
        imageDesc.format.layout = ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8;
        imageDesc.format.type = ZE_IMAGE_FORMAT_TYPE_UINT;
        imageDesc.width = 256;
        imageDesc.height = 256;
        imageDesc.depth = 1;
        ze_image_handle_t image;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageCreate(context, devices[0], &imageDesc, &image));
        EXPECT_TRUE(compare_env("zeImageCreate", std::to_string(i + 1)));

        ze_image_properties_t imageProps = {ZE_STRUCTURE_TYPE_IMAGE_PROPERTIES};
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageGetProperties(devices[0], &imageDesc, &imageProps));
        EXPECT_TRUE(compare_env("zeImageGetProperties", std::to_string(i + 1)));

        ze_image_allocation_ext_properties_t allocProps = {ZE_STRUCTURE_TYPE_IMAGE_ALLOCATION_EXT_PROPERTIES};
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageGetAllocPropertiesExt(context, image, &allocProps));
        EXPECT_TRUE(compare_env("zeImageGetAllocPropertiesExt", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageDestroy(image));
        EXPECT_TRUE(compare_env("zeImageDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingSamplerApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeSampler APIs
        ze_sampler_desc_t samplerDesc = {ZE_STRUCTURE_TYPE_SAMPLER_DESC};
        samplerDesc.addressMode = ZE_SAMPLER_ADDRESS_MODE_CLAMP;
        samplerDesc.filterMode = ZE_SAMPLER_FILTER_MODE_NEAREST;
        ze_sampler_handle_t sampler;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerCreate(context, devices[0], &samplerDesc, &sampler));
        EXPECT_TRUE(compare_env("zeSamplerCreate", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeSamplerDestroy(sampler));
        EXPECT_TRUE(compare_env("zeSamplerDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFenceApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        ze_command_queue_desc_t queueDesc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC};
        ze_command_queue_handle_t queue;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, devices[0], &queueDesc, &queue));

        // zeFence APIs
        ze_fence_desc_t fenceDesc = {ZE_STRUCTURE_TYPE_FENCE_DESC};
        ze_fence_handle_t fence;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceCreate(queue, &fenceDesc, &fence));
        EXPECT_TRUE(compare_env("zeFenceCreate", std::to_string(i + 1)));

        zeFenceQueryStatus(fence);
        EXPECT_TRUE(compare_env("zeFenceQueryStatus", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceReset(fence));
        EXPECT_TRUE(compare_env("zeFenceReset", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceHostSynchronize(fence, 0));
        EXPECT_TRUE(compare_env("zeFenceHostSynchronize", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFenceDestroy(fence));
        EXPECT_TRUE(compare_env("zeFenceDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(queue));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingPhysicalMemoryApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zePhysicalMem APIs
        ze_physical_mem_desc_t physMemDesc = {ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC};
        physMemDesc.size = 1024 * 1024; // 1MB
        ze_physical_mem_handle_t physMem;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemCreate(context, devices[0], &physMemDesc, &physMem));
        EXPECT_TRUE(compare_env("zePhysicalMemCreate", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemDestroy(context, physMem));
        EXPECT_TRUE(compare_env("zePhysicalMemDestroy", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingVirtualMemoryApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // zeVirtualMem APIs
        size_t pageSize = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemQueryPageSize(context, devices[0], 1024, &pageSize));
        EXPECT_TRUE(compare_env("zeVirtualMemQueryPageSize", std::to_string(i + 1)));

        void *virtualAddr = nullptr;
        size_t size = pageSize;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemReserve(context, nullptr, size, &virtualAddr));
        EXPECT_TRUE(compare_env("zeVirtualMemReserve", std::to_string(i + 1)));

        ze_memory_access_attribute_t access = ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE;
        size_t accessSize = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemGetAccessAttribute(context, virtualAddr, size, &access, &accessSize));
        EXPECT_TRUE(compare_env("zeVirtualMemGetAccessAttribute", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemSetAccessAttribute(context, virtualAddr, size, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE));
        EXPECT_TRUE(compare_env("zeVirtualMemSetAccessAttribute", std::to_string(i + 1)));

        // Physical memory for mapping
        ze_physical_mem_desc_t physMemDesc = {ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC};
        physMemDesc.size = size;
        ze_physical_mem_handle_t physMem;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemCreate(context, devices[0], &physMemDesc, &physMem));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemMap(context, virtualAddr, size, physMem, 0, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE));
        EXPECT_TRUE(compare_env("zeVirtualMemMap", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemUnmap(context, virtualAddr, size));
        EXPECT_TRUE(compare_env("zeVirtualMemUnmap", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeVirtualMemFree(context, virtualAddr, size));
        EXPECT_TRUE(compare_env("zeVirtualMemFree", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zePhysicalMemDestroy(context, physMem));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingFabricApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        // zeFabricVertex APIs
        ze_fabric_vertex_handle_t fabricVertex;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetFabricVertexExp(devices[0], &fabricVertex));
        EXPECT_TRUE(compare_env("zeDeviceGetFabricVertexExp", std::to_string(i + 1)));

        uint32_t fabricVertexCount = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricVertexGetExp(drivers[i], &fabricVertexCount, nullptr));
        EXPECT_TRUE(compare_env("zeFabricVertexGetExp", std::to_string(i + 1)));

        ze_fabric_vertex_exp_properties_t fabricVertexProps = {ZE_STRUCTURE_TYPE_FABRIC_VERTEX_EXP_PROPERTIES};
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricVertexGetPropertiesExp(fabricVertex, &fabricVertexProps));
        EXPECT_TRUE(compare_env("zeFabricVertexGetPropertiesExp", std::to_string(i + 1)));

        ze_device_handle_t fabricDevice;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricVertexGetDeviceExp(fabricVertex, &fabricDevice));
        EXPECT_TRUE(compare_env("zeFabricVertexGetDeviceExp", std::to_string(i + 1)));

        uint32_t subVertexCount = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricVertexGetSubVerticesExp(fabricVertex, &subVertexCount, nullptr));
        EXPECT_TRUE(compare_env("zeFabricVertexGetSubVerticesExp", std::to_string(i + 1)));

        // zeFabricEdge APIs
        uint32_t edgeCount = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricEdgeGetExp(fabricVertex, fabricVertex, &edgeCount, nullptr));
        EXPECT_TRUE(compare_env("zeFabricEdgeGetExp", std::to_string(i + 1)));

        ze_fabric_edge_exp_properties_t edgeProps = {ZE_STRUCTURE_TYPE_FABRIC_EDGE_EXP_PROPERTIES};
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricEdgeGetPropertiesExp(nullptr, &edgeProps));
        EXPECT_TRUE(compare_env("zeFabricEdgeGetPropertiesExp", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeFabricEdgeGetVerticesExp(nullptr, &fabricVertex, &fabricVertex));
        EXPECT_TRUE(compare_env("zeFabricEdgeGetVerticesExp", std::to_string(i + 1)));
      }
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingRTASApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      // zeRTASBuilder APIs
      ze_rtas_builder_exp_desc_t builderDesc = {ZE_STRUCTURE_TYPE_RTAS_BUILDER_EXP_DESC};
      ze_rtas_builder_exp_handle_t rtasBuilder;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASBuilderCreateExp(drivers[i], &builderDesc, &rtasBuilder));
      EXPECT_TRUE(compare_env("zeRTASBuilderCreateExp", std::to_string(i + 1)));

      ze_rtas_builder_build_op_exp_desc_t buildOpDesc = {ZE_STRUCTURE_TYPE_RTAS_BUILDER_BUILD_OP_EXP_DESC};
      ze_rtas_builder_exp_properties_t builderProps = {ZE_STRUCTURE_TYPE_RTAS_BUILDER_EXP_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASBuilderGetBuildPropertiesExp(rtasBuilder, &buildOpDesc, &builderProps));
      EXPECT_TRUE(compare_env("zeRTASBuilderGetBuildPropertiesExp", std::to_string(i + 1)));

      void *scratchBuffer = nullptr;
      void *rtasBuffer = nullptr;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASBuilderBuildExp(rtasBuilder, &buildOpDesc, scratchBuffer, 1024, rtasBuffer, 1024, nullptr, nullptr, nullptr, nullptr));
      EXPECT_TRUE(compare_env("zeRTASBuilderBuildExp", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASBuilderDestroyExp(rtasBuilder));
      EXPECT_TRUE(compare_env("zeRTASBuilderDestroyExp", std::to_string(i + 1)));

      // zeRTASParallelOperation APIs
      ze_rtas_parallel_operation_exp_handle_t parallelOp;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASParallelOperationCreateExp(drivers[i], &parallelOp));
      EXPECT_TRUE(compare_env("zeRTASParallelOperationCreateExp", std::to_string(i + 1)));

      ze_rtas_parallel_operation_exp_properties_t parallelProps = {ZE_STRUCTURE_TYPE_RTAS_PARALLEL_OPERATION_EXP_PROPERTIES};
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASParallelOperationGetPropertiesExp(parallelOp, &parallelProps));
      EXPECT_TRUE(compare_env("zeRTASParallelOperationGetPropertiesExp", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASParallelOperationJoinExp(parallelOp));
      EXPECT_TRUE(compare_env("zeRTASParallelOperationJoinExp", std::to_string(i + 1)));

      EXPECT_EQ(ZE_RESULT_SUCCESS, zeRTASParallelOperationDestroyExp(parallelOp));
      EXPECT_TRUE(compare_env("zeRTASParallelOperationDestroyExp", std::to_string(i + 1)));

      // Driver RTAS compatibility check
      ze_rtas_format_exp_t formatA = ZE_RTAS_FORMAT_EXP_INVALID;
      ze_rtas_format_exp_t formatB = ZE_RTAS_FORMAT_EXP_INVALID;
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDriverRTASFormatCompatibilityCheckExp(drivers[i], formatA, formatB));
      EXPECT_TRUE(compare_env("zeDriverRTASFormatCompatibilityCheckExp", std::to_string(i + 1)));
    }
  }

  TEST(
      CoreApiLoaderDriverInteraction,
      GivenLevelZeroLoaderPresentWhenCallingExperimentalApisThenExpectNullDriverIsReachedSuccessfully)
  {
    uint32_t pInitDriversCount = 0;
    ze_init_driver_type_desc_t desc = {ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC};
    desc.flags = UINT32_MAX;
    desc.pNext = nullptr;
    std::vector<ze_driver_handle_t> drivers;
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, nullptr, &desc));
    drivers.resize(pInitDriversCount);
    EXPECT_EQ(ZE_RESULT_SUCCESS, zeInitDrivers(&pInitDriversCount, drivers.data(), &desc));
    EXPECT_GT(pInitDriversCount, 0);

    for (std::size_t i = 0; i < drivers.size(); i++)
    {
      uint32_t deviceCount = 1;
      std::vector<ze_device_handle_t> devices(deviceCount);
      EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGet(drivers[i], &deviceCount, devices.data()));

      {
        ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC};
        ze_context_handle_t context;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextCreate(drivers[i], &contextDesc, &context));

        // Experimental Memory APIs
        void *ptr = nullptr;
        size_t size = 1024;
        ze_memory_atomic_attr_exp_flags_t atomicAttr;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemGetAtomicAccessAttributeExp(context, devices[0], ptr, size, &atomicAttr));
        EXPECT_TRUE(compare_env("zeMemGetAtomicAccessAttributeExp", std::to_string(i + 1)));

        EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemSetAtomicAccessAttributeExp(context, devices[0], ptr, size, ZE_MEMORY_ATOMIC_ATTR_EXP_FLAG_SYSTEM_ATOMICS));
        EXPECT_TRUE(compare_env("zeMemSetAtomicAccessAttributeExp", std::to_string(i + 1)));

        // Experimental Kernel APIs
        ze_module_desc_t moduleDesc = {ZE_STRUCTURE_TYPE_MODULE_DESC};
        ze_module_handle_t module;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, devices[0], &moduleDesc, &module, nullptr));

        ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC};
        kernelDesc.pKernelName = "test";
        ze_kernel_handle_t kernel;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelCreate(module, &kernelDesc, &kernel));

        uint32_t offsetX = 0, offsetY = 0, offsetZ = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetGlobalOffsetExp(kernel, offsetX, offsetY, offsetZ));
        EXPECT_TRUE(compare_env("zeKernelSetGlobalOffsetExp", std::to_string(i + 1)));

        size_t kernelSize = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelGetBinaryExp(kernel, &kernelSize, nullptr));
        EXPECT_TRUE(compare_env("zeKernelGetBinaryExp", std::to_string(i + 1)));

        // Experimental Image APIs
        ze_image_desc_t imageDesc = {ZE_STRUCTURE_TYPE_IMAGE_DESC};
        imageDesc.type = ZE_IMAGE_TYPE_2D;
        imageDesc.format.layout = ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8;
        imageDesc.format.type = ZE_IMAGE_FORMAT_TYPE_UINT;
        imageDesc.width = 256;
        imageDesc.height = 256;
        imageDesc.depth = 1;
        ze_image_handle_t image;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageCreate(context, devices[0], &imageDesc, &image));

        uint64_t deviceOffset = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageGetDeviceOffsetExp(image, &deviceOffset));
        EXPECT_TRUE(compare_env("zeImageGetDeviceOffsetExp", std::to_string(i + 1)));

        // Experimental Event APIs
        ze_event_pool_desc_t eventPoolDesc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC};
        eventPoolDesc.count = 1;
        ze_event_pool_handle_t eventPool;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &eventPoolDesc, 0, nullptr, &eventPool));

        ze_event_desc_t eventDesc = {ZE_STRUCTURE_TYPE_EVENT_DESC};
        ze_event_handle_t event;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventCreate(eventPool, &eventDesc, &event));

        uint32_t timestampCount = 0;
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventQueryTimestampsExp(event, devices[0], &timestampCount, nullptr));
        EXPECT_TRUE(compare_env("zeEventQueryTimestampsExp", std::to_string(i + 1)));

        // Cleanup
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolDestroy(eventPool));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeImageDestroy(image));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module));
        EXPECT_EQ(ZE_RESULT_SUCCESS, zeContextDestroy(context));
      }
    }
  }

} // namespace
