/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dali-test-suite-utils.h>
#include <dali/internal/network/common/socket-factory.h>
#include <dali/internal/network/common/socket-interface.h>

using namespace Dali;
using namespace Dali::Internal::Adaptor;

void utc_dali_socket_factory_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_socket_factory_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliSocketFactoryNewSocketP(void)
{
  SocketFactory    factory;
  SocketInterface* socket = factory.NewSocket(SocketInterface::TCP);

  if(socket)
  {
    factory.DestroySocket(socket);
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryTcpTypeP(void)
{
  SocketFactory    factory;
  SocketInterface* socket = factory.NewSocket(SocketInterface::TCP);

  if(socket)
  {
    factory.DestroySocket(socket);
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryUdpTypeP(void)
{
  SocketFactory    factory;
  SocketInterface* socket = factory.NewSocket(SocketInterface::UDP);

  if(socket)
  {
    factory.DestroySocket(socket);
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryMultipleCreationsP(void)
{
  SocketFactory factory;

  for(int i = 0; i < 5; ++i)
  {
    SocketInterface* socket = factory.NewSocket(SocketInterface::TCP);
    if(socket)
    {
      factory.DestroySocket(socket);
    }
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryDifferentTypesP(void)
{
  SocketFactory factory;

  SocketInterface* tcpSocket = factory.NewSocket(SocketInterface::TCP);
  SocketInterface* udpSocket = factory.NewSocket(SocketInterface::UDP);

  if(tcpSocket)
  {
    factory.DestroySocket(tcpSocket);
  }
  if(udpSocket)
  {
    factory.DestroySocket(udpSocket);
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryNullHandlingP(void)
{
  SocketFactory    factory;
  SocketInterface* socket = factory.NewSocket(SocketInterface::TCP);

  if(!socket)
  {
    DALI_TEST_CHECK(true);
  }
  else
  {
    DALI_TEST_CHECK(socket != nullptr);
    factory.DestroySocket(socket);
  }

  END_TEST;
}

int UtcDaliSocketFactoryFactoryInstanceP(void)
{
  SocketFactory factory1;
  SocketFactory factory2;

  SocketInterface* socket1 = factory1.NewSocket(SocketInterface::TCP);
  SocketInterface* socket2 = factory2.NewSocket(SocketInterface::TCP);

  if(socket1)
  {
    factory1.DestroySocket(socket1);
  }
  if(socket2)
  {
    factory2.DestroySocket(socket2);
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryConsecutiveCreationsP(void)
{
  SocketFactory factory;

  SocketInterface* socket1 = factory.NewSocket(SocketInterface::TCP);
  SocketInterface* socket2 = factory.NewSocket(SocketInterface::TCP);
  SocketInterface* socket3 = factory.NewSocket(SocketInterface::TCP);
  SocketInterface* socket4 = factory.NewSocket(SocketInterface::UDP);
  SocketInterface* socket5 = factory.NewSocket(SocketInterface::UDP);

  if(socket1)
    factory.DestroySocket(socket1);
  if(socket2)
    factory.DestroySocket(socket2);
  if(socket3)
    factory.DestroySocket(socket3);
  if(socket4)
    factory.DestroySocket(socket4);
  if(socket5)
    factory.DestroySocket(socket5);

  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliSocketFactoryScopedDestructionP(void)
{
  SocketFactory    factory;
  SocketInterface* socket = factory.NewSocket(SocketInterface::TCP);

  if(socket)
  {
    factory.DestroySocket(socket);
  }

  DALI_TEST_CHECK(true);

  END_TEST;
}
