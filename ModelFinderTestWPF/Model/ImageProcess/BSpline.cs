using Emgu.CV;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace ModelFinder.Model.ImageProcess
{
    public class BSpline
    {
        public int m_nDegree;
        public List<float> m_thida;
        public List<float> m_vecKnots;

        public List<PointF> DoCurveFitting(ref List<PointF> vecFitPoints)
        {
            var outputcurve = new List<PointF>();
            m_vecKnots = new List<float>();
            m_thida = new List<float>();
            try
            {
                int k = 3;
                int n = vecFitPoints.Count - 1;

                for (int i = 0; i <= (n + 1) + k; i++)
                {
                    if (i <= k)
                        m_vecKnots.Add(0);
                    else if (i > k && i < n + 1)
                    {
                        m_vecKnots.Add(i - k);
                    }
                    else if (i >= n + 1)
                    {
                        m_vecKnots.Add((n + 1) - k);
                    }
                }

                for (float t = m_vecKnots[k]; t < m_vecKnots[n + 1]; t += (float)0.10)
                {
                    float px_t = 0;
                    float py_t = 0;
                    for (int i = 0; i <= n; i++)
                    {
                        px_t += vecFitPoints[i].X * N_Basis(i, k, t);
                        py_t += vecFitPoints[i].Y * N_Basis(i, k, t);
                    }
                    outputcurve.Add( new PointF(px_t, py_t));
                }

                GetThida(outputcurve);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
            return outputcurve;
        }

        private void GetThida(List<PointF> p)
        {
            float thida = 0;
            try
            {
                for (int i = 0; i < p.Count - 1; i++)
                {
                    if ((p[i + 1].X + p[i].X) != 0)
                    {
                        thida = (float)(90 - Math.Atan((p[i + 1].Y - p[i].Y) / (-p[i + 1].Y + p[i].Y)) * 180.0 / 3.14159);
                    }
                    else
                        thida = 0;
                    m_thida.Add(thida);
                }
                m_thida.Add(thida);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private float N_Basis(int i, int k, float t)
        {
            float ans = 0;
            try
            {
                if (k == 0)
                {
                    if (t >= m_vecKnots[i] && t < m_vecKnots[i + 1])
                        return 1;
                    else
                        return 0;
                }
                else
                {
                    float term1 = 0, term2 = 0;
                    if ((m_vecKnots[i + k] - m_vecKnots[i]) == 0)
                    {
                        term1 = 0;
                    }
                    else
                    {
                        term1 = (t - m_vecKnots[i]) / (m_vecKnots[i + k] - m_vecKnots[i]);
                    }

                    if ((m_vecKnots[i + 1 + k] - m_vecKnots[i + 1]) == 0)
                    {
                        term2 = 0;
                    }
                    else
                    {
                        term2 = (m_vecKnots[i + 1 + k] - t) / (m_vecKnots[i + 1 + k] - m_vecKnots[i + 1]);
                    }
                    ans = term1 * N_Basis(i, k - 1, t) + term2 * N_Basis(i + 1, k - 1, t);
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
            return ans;
        }
    }
}
