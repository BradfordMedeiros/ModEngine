(define texturename "graphs-testplot")
(define xRangeSecs 10)
(define yRange 400)
(define gridLines (list 0.5 0 -0.5))
(define statname "fps")
(define minDrawingTime 0.05)


(define (create-obj)
	(format #t "create obj placeholder\n")
   	(mk-obj-attr "someobj"     
  		(list
  			(list "position" (list 1 1 0))
  			(list "mesh" "./res/models/box/spriteplane.dae")
  			(list "rotation" (list 0 0 1 180))
  		)
	)
  (gameobj-setattr! (lsobj-name "someobj/Plane") 
  	(list
  		(list "texture" texturename)
  	)
  )
)


(define (convertX value) (- (* 2 (/ (floor-remainder value xRangeSecs) xRangeSecs)) 1))
(define (convertY value) (min 1 (max -1 (- (* 2 (/ value yRange)) 1))))
(define (unconvertY value) (* yRange (* 0.5 (- value 1))))

(define pointX 0)
(define pointY 0)
(define (plotPoint x y)
	(define firstPoint (< x pointX))
	(if (and pointX pointY (not firstPoint))
   	(draw-line (list pointX pointY 100) (list x y 100) #f textureId)
   	(refreshGraph)
	)
	(set! pointX x)
	(set! pointY y)
)

(define (draw-grid-line yloc) (draw-line (list -1 yloc 0) (list 1 yloc 0) #f textureId))

(define (convert res value) (* 0.5 res (+ value 1)))
(define (label-grid-line yloc)
	(draw-text (number->string (inexact->exact (round (unconvertY yloc)))) 10  (+ 20 (convert 1000 yloc)) 6 #f textureId)
	(draw-text (number->string (inexact->exact (round (unconvertY yloc)))) 950 (+ 20 (convert 1000 yloc)) 6 #f textureId)
)
(define (draw-grid) 
	(define yLoc gridLines)
	(map draw-grid-line yLoc)
	(map label-grid-line yLoc)
)

(define lastTime (time-seconds))
(define (onFrame)
	(define now (time-seconds))
	(define diff (- now lastTime))
	(if (> diff minDrawingTime)
		(begin
			(set! lastTime now)
      (let* ((currSec (time-seconds)) (fps (runstat statname)) (coordX (convertX currSec)) (coordY (convertY fps)))
				;(format #t "(time = ~a[~a] , fps = ~a[~a])" currSec  coordX fps coordY)
				(plotPoint coordX coordY)
			)
		)
	)
)



(define (addPermaData)
	(draw-grid)
	(draw-text "Performance - Graph of Framerate" 20 30 4 #f textureId)
	(draw-text statname 950 30 4 #f textureId)
	(draw-line (list -1 0.9 0) (list 1 0.9 0) #f textureId)
	(draw-line (list -1 1 0) (list 1 1 0)  #f textureId)
)

(define (refreshGraph)
	(clear-texture textureId )
	(addPermaData)
)
(define (onKeyChar codepoint)
	(format #t "codepoint: ~a\n" codepoint)
	(if (equal? codepoint 44) (refreshGraph)) 
	(if (equal? codepoint 47) (create-obj))                        
)

(define textureId #f)
(define texturename texturename)
(set! textureId (create-texture texturename 1000 1000))
(addPermaData)

