# Introduction
This project is an MFC application for automatically transcribing the CAN Network Database(dbc) file into program source files. 
The UI interface is easy to use for anyone who is developing the CAN protocol.

# Copyright
Feel free to download the project and modify it as you like. But notice that the author(joeccmou@qq.com) has all the copyrights.
	
	> IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	> FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	> DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	> SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	> CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	> OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	> OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Release Note
Date: 2019/3/20 22:10:03   
the automatic code generation function works. Only for Intel order(little endian) right now.

	
# User Guidance
1. Output Message Structure and Initialization Function
	> step1: click "File"-->"Open..." to open a .dbc file
	>  
	> step2: click "Output"-->"Source Code (Initialize Signals)"-->"C language", the XXX(init).c source file will be output to the path where the dbc file is located
	
2. Output Read Message Function
	> step1: click "File"-->"Open..." to open a .dbc file
	>  
	> step2: click "Output"-->"Source Code (Read Signals)"-->"C language", the XXX(Read).c source file will be output to the path where the dbc file is located	
	
3. Output Write Message Function
	> step1: click "File"-->"Open..." to open a .dbc file
	>  
	> step2: click "Output"-->"Source Code (Write Signals)"-->"C language", the XXX(Write).c source file will be output to the path where the dbc file is located	
	
