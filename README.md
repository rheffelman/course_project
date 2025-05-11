# Download the latest 64-bit version of SFML

    https://www.sfml-dev.org/files/SFML-2.6.1-windows-vc17-64-bit.zip
    
# Extract SFML to a folder on your computer with NO SPACES

    I highly recommend creating a folder c:\dev\libraries and extracting there

    Resulting in c:\dev\libraries\SFML-2.6.1\ as the SFML dir

# Create a Windows Environment Variable used by VS to point to SFML include dir:

    Click Start menu

    Type 'env' and click 'Edit the environment variables for your account'

    Click 'New...' and create:

        Name: SFML_DIR

        Value: C:\dev\libraries\SFML-2.6.1

# You now need to tell Windows where to find the SFML .dll files by adding them to your 'Path' environment variable

    Click Start menu

    Type 'env' and click 'Edit the environment variables for your account'

    Click the 'Path' variable and click 'Edit...'

    Click 'New' on the right hand side

    Type %SFML_DIR%\bin

    Click 'Ok' to exit

    Restart Visual Studio if it is open

# Download and extract the assignment zip file

# Double click 'SFMLGame.sln', Visual Studio should launch

# Click 'ok' to any warnings

# In Visual Studio, switch from 'Debug' to 'Release' then right click SFMLGame project and click "Build"

# If everything was successful, no errors should pop up

# Common Windows Errors:

    If it says SFML\Graphics.hpp not found, you didn't do steps 4 correctly

    If it says DLL not found, you didn't do step 5 correctly
