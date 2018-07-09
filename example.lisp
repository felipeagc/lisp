(progn
  (defun fat (n)
    (if (= n 1)
	1
      (* n (fat (- n 1)))))
  (print (fat 5)))
