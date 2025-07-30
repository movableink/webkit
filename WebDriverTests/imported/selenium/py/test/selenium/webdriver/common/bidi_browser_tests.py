# Licensed to the Software Freedom Conservancy (SFC) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The SFC licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import pytest

from selenium.webdriver.common.bidi.browser import ClientWindowInfo
from selenium.webdriver.common.bidi.browser import ClientWindowState


@pytest.mark.xfail_safari
def test_browser_initialized(driver):
    """Test that the browser module is initialized properly."""
    assert driver.browser is not None


@pytest.mark.xfail_safari
def test_create_user_context(driver):
    """Test creating a user context."""
    user_context = driver.browser.create_user_context()
    assert user_context is not None

    # Clean up
    driver.browser.remove_user_context(user_context)


@pytest.mark.xfail_safari
def test_get_user_contexts(driver):
    """Test getting user contexts."""
    user_context1 = driver.browser.create_user_context()
    user_context2 = driver.browser.create_user_context()

    user_contexts = driver.browser.get_user_contexts()
    # it should be 3 since there is a default user context present, therefore >= 2
    assert len(user_contexts) >= 2

    # Clean up
    driver.browser.remove_user_context(user_context1)
    driver.browser.remove_user_context(user_context2)


@pytest.mark.xfail_safari
def test_remove_user_context(driver):
    """Test removing a user context."""
    user_context1 = driver.browser.create_user_context()
    user_context2 = driver.browser.create_user_context()

    user_contexts = driver.browser.get_user_contexts()
    assert len(user_contexts) >= 2

    driver.browser.remove_user_context(user_context2)

    updated_user_contexts = driver.browser.get_user_contexts()
    assert user_context1 in updated_user_contexts
    assert user_context2 not in updated_user_contexts

    # Clean up
    driver.browser.remove_user_context(user_context1)


@pytest.mark.xfail_safari
def test_get_client_windows(driver):
    """Test getting client windows."""
    client_windows = driver.browser.get_client_windows()

    assert client_windows is not None
    assert len(client_windows) > 0

    window_info = client_windows[0]
    assert isinstance(window_info, ClientWindowInfo)
    assert window_info.get_client_window() is not None
    assert window_info.get_state() is not None
    assert isinstance(window_info.get_state(), str)
    assert window_info.get_width() > 0
    assert window_info.get_height() > 0
    assert isinstance(window_info.is_active(), bool)
    assert window_info.get_x() >= 0
    assert window_info.get_y() >= 0


@pytest.mark.xfail_safari
def test_raises_exception_when_removing_default_user_context(driver):
    with pytest.raises(Exception):
        driver.browser.remove_user_context("default")


@pytest.mark.xfail_safari
def test_client_window_state_constants(driver):
    assert ClientWindowState.FULLSCREEN == "fullscreen"
    assert ClientWindowState.MAXIMIZED == "maximized"
    assert ClientWindowState.MINIMIZED == "minimized"
    assert ClientWindowState.NORMAL == "normal"
